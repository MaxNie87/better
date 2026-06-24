# POI 导入任务设计（后端）

本文档面向后端开发，描述露营地点的 POI 批量导入任务、字段映射、去重策略和活跃状态更新机制。MVP 阶段小程序仍使用 `data/camp-sites.js` 中的静态数据，本设计为后端联调与未来落地提供蓝图。

## 一、目标与原则

- 通过合规的位置服务 API（如腾讯位置服务、高德 Web 服务）获取候选 POI 数据。
- 不在小程序端直接抓取第三方网站页面；批量获取统一在后端任务中完成，并遵守目标 API 的服务条款。
- 数据写入后台数据库后，通过统一接口供小程序消费，小程序端只关心结构化字段。

## 二、整体流程

1. 任务调度：定时触发（如每日一次）或人工触发；按行政区/经纬度网格切片，避免单次请求过大。
2. 候选 POI 拉取：按关键词（“露营地”“营地”“露营基地”等）+ 区域参数调用位置服务搜索接口。
3. 字段标准化：把第三方字段映射成统一模型（见下文“字段映射”）。
4. 去重与合并：根据外部 POI ID、坐标、名称做多级判重，决定新增、更新或跳过。
5. 活跃状态评估：结合距离上次出现时间、用户反馈、API 返回的状态字段，更新 `isActive`。
6. 入库与版本：写入数据库并保留导入快照（便于追溯/回滚）。
7. 接口暴露：列表、详情、筛选接口对小程序开放，字段保持与 MVP `site-service` 一致。

## 三、字段映射

后端统一模型字段（与 `data/camp-sites.js` 中的字段一致，便于小程序无缝切换）：

| 内部字段 | 类型 | 来源建议 | 备注 |
| --- | --- | --- | --- |
| `id` | string | 后端自增/雪花 ID | 与外部 POI 解耦 |
| `name` | string | API: `name` / `title` | 必填 |
| `description` | string | API: `description` / `intro` / 类目说明 | 可空 |
| `note` | string | 用户反馈 / 运营补充 | MVP 仅来自反馈 |
| `latitude` | number | API 经纬度 | 注意坐标系（统一为 GCJ02 适配微信地图） |
| `longitude` | number | API 经纬度 | 同上 |
| `address` | string | API: `address` / 行政区 + 详细地址 | 必填 |
| `seasons` | string[] | 运营标注或文本提取 | 取值：spring/summer/autumn/winter |
| `hasToilet` | boolean \| null | 设施字段 / 用户反馈 | null 表示未知 |
| `isActive` | boolean \| null | 综合判定（见“活跃状态”） | null 表示未知 |
| `tags` | string[] | API 类目 + 运营标签 | 用于展示与搜索 |
| `source` | string | `poi_amap` / `poi_tencent` / `manual` | 区分数据来源 |
| `externalPoiId` | string \| null | API: POI 唯一 ID | 去重核心字段 |
| `updatedAt` | string (ISO) | 入库或更新时间 | 用于排序与展示 |
| `coverImage` | string | API: `photos` 第一张 / 运营上传 | MVP 可为空 |

字段映射示例（伪代码）：

```js
function mapAmapPoi(poi) {
  return {
    name: poi.name,
    description: poi.type || '',
    latitude: parseFloat(poi.location.split(',')[1]),
    longitude: parseFloat(poi.location.split(',')[0]),
    address: poi.pname + poi.cityname + poi.adname + (poi.address || ''),
    seasons: [],
    hasToilet: null,
    isActive: null,
    tags: poi.type ? poi.type.split(';') : [],
    source: 'poi_amap',
    externalPoiId: 'amap_' + poi.id,
    updatedAt: new Date().toISOString(),
    coverImage: (poi.photos && poi.photos[0] && poi.photos[0].url) || ''
  };
}
```

## 四、去重策略（多级判重）

按以下顺序判断候选 POI 与既有数据的关系：

1. 强匹配 - 外部 POI ID：`source + externalPoiId` 完全相同，视为同一记录，做更新。
2. 中匹配 - 坐标 + 名称：
   - 距离 ≤ 50 米且名称相似度 ≥ 0.85（推荐 Jaro-Winkler 或归一化 Levenshtein）；
   - 视为同一营地，记录新的来源/外部 ID（多来源合并到同一条）。
3. 弱匹配 - 名称完全相同 + 距离 ≤ 200 米：
   - 触发人工审核队列，不自动合并；
   - 审核通过后再做合并或新增。
4. 未命中：作为新记录入库，`source = poi_xxx`，初始 `hasToilet = null`、`isActive = null`。

合并规则：

- `source` 升级为多来源标记（如 `poi_amap+poi_tencent`），或新增 `sourceList` 数组保留全部来源。
- `tags` 取并集，并去重。
- `hasToilet`、`isActive` 优先保留用户反馈值，否则保留更近的导入值。
- `updatedAt` 总是更新为最近一次写入时间。

## 五、活跃状态（isActive）更新

活跃状态难以从 POI API 直接得到，采用综合策略：

- 用户反馈优先：来自 `feedback` 接口的 `isActive` 在 90 天内视为权威值。
- 持续命中：连续 N 次（如 3 次）导入仍能搜索到 → 维持 `isActive = true`。
- 长时间缺失：若已存在记录连续 M 次（如 5 次）导入未出现 → 标记 `isActive = false`，并在详情页显示“近期未在公开数据中出现”。
- 硬下线：API 明确返回 `status = closed` / `closed = true` 时立即置为 `false`。
- 未知态：刚导入且无反馈、无连续命中证据时保持 `null`，前端展示“状态未知”。

建议数据库为每条记录维护：

- `lastSeenAt`：最近一次在导入中出现的时间。
- `missCount`：累计未命中次数。
- `feedbackUpdatedAt`：最近一次用户反馈时间。

## 六、接口草案（供小程序消费）

- `GET /api/sites?hasToilet=&isActive=&favorite=&keyword=&season=`
  - 返回字段与 `applyOverrides` 输出一致；
  - 支持分页参数 `page`、`pageSize`。
- `GET /api/sites/:id`
  - 返回单个营地详情，包含 `source`、`externalPoiId`、`updatedAt`。
- `POST /api/sites/:id/feedback`
  - body: `{ hasToilet?: boolean, isActive?: boolean, note?: string }`
  - 写入反馈表，并触发活跃状态重算。
- `POST /api/favorites/:id` / `DELETE /api/favorites/:id`
  - 用户态收藏（MVP 阶段小程序内本地缓存，未来登录后迁移到后端）。

## 七、合规与风险

- 所有第三方 API 必须使用平台官方授权；导入频率遵守服务条款。
- 不抓取无授权的网页或 App 数据；如需引入，需法务确认。
- 用户反馈中包含的位置/备注信息需经过基础敏感词过滤后入库。
- 对外展示字段中如包含第三方版权图片，需保留来源标注。

## 八、与小程序对接顺序

1. 后端按本设计实现接口与导入任务，先以管理后台填充种子数据。
2. 在小程序中新增 `utils/api.js`，封装 `wx.request`，并将 `site-service.js` 的 `listSites/getSiteById/submitFeedback` 切换为远程实现。
3. 保持 `data/camp-sites.js` 作为离线兜底数据，便于无网络场景调试。
