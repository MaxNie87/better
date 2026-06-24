# 营地图 · 露营地点管理小程序（MVP）

这是一个用于记录、查看和管理露营地点的原生微信小程序 MVP。第一版使用本地 Mock 数据驱动，后端接口接入 POI 导入与用户反馈后再做替换。

## 功能概览

- 地图首页：当前位置定位、营地 Marker、点击 Marker 弹出地点卡片、跳转详情。
- 地点列表：以列表方式浏览全部营地，支持「有卫生间 / 活跃营地 / 已收藏」筛选与关键字搜索。
- 地点详情：展示基础信息、设施、来源、最近更新时间，支持收藏与跳转纠错。
- 信息纠错：用户提交卫生间、是否仍在使用、备注等补充信息，MVP 阶段写入小程序本地存储并叠加到展示数据。
- POI 导入预留：`docs/poi_import.md` 详细说明后端导入任务、字段映射、去重与活跃状态更新策略。

## 目录结构

```
camp_site_miniprogram/
├── app.js / app.json / app.wxss / sitemap.json
├── project.config.json / project.private.config.json
├── data/
│   └── camp-sites.js          # MVP 模拟营地数据
├── docs/
│   └── poi_import.md          # 后端 POI 导入任务设计
├── pages/
│   ├── map/                   # 地图首页
│   ├── sites/                 # 营地列表
│   ├── site-detail/           # 营地详情
│   └── feedback/              # 补充/纠错表单
├── utils/
│   ├── site-service.js        # 数据读取/筛选/收藏/反馈封装
│   └── format.js              # 通用格式化工具
└── README.md
```

## 运行前准备

1. 微信开发者工具：从 [微信官方下载](https://developers.weixin.qq.com/miniprogram/dev/devtools/download.html) 安装最新稳定版。
2. 微信小程序 AppID：
   - 正式开发请使用自有小程序 AppID；
   - 暂无 AppID 时可在微信开发者工具中选择「测试号」模式直接打开本项目。
3. 位置服务：
   - MVP 阶段直接使用微信内置 `map` 组件即可，无需腾讯/高德 Web 服务 Key；
   - 如需后续启用搜索 POI、路线规划等能力，再申请 [腾讯位置服务 Key](https://lbs.qq.com/) 或 [高德开放平台 Key](https://lbs.amap.com/)。
4. 后端导入任务用到的位置服务 API Key：
   - 后端实现时再选择具体厂商，需结合服务条款与授权范围确认；
   - 详见 `docs/poi_import.md`。

## 启动步骤

1. 打开微信开发者工具，选择「导入项目」。
2. 项目目录指向本仓库中的 `camp_site_miniprogram/` 文件夹。
3. AppID 选择自己的或选择「测试号」。
4. 工具自动加载后即可在模拟器内查看：
   - 默认进入「地图」页（成都附近示例数据）；
   - 切换到「营地」Tab 浏览列表与筛选；
   - 任意营地 → 详情 → 「补充/纠错」体验反馈写入。
5. 真机预览：在开发者工具点击「预览」并扫码。

## 数据与状态说明

- 营地数据：`data/camp-sites.js`，包含 6 个示例点位，覆盖「有/无卫生间」「活跃/停用/未知」等状态组合，便于联调筛选逻辑。
- 收藏状态：保存在小程序 `wx.setStorageSync('favoriteIds', [...])`，应用启动时读入 `globalData`。
- 用户反馈：保存在 `wx.setStorageSync('feedbackOverrides', { siteId: { hasToilet, isActive, note, updatedAt } })`，列表/详情页会自动叠加到原始数据上展示。
- `site-service.js` 提供 `listSites / getSiteById / submitFeedback`，未来切换为后端接口时仅替换内部实现，调用方无需变更。

## 后续路线

- 接入真实后端：用 `wx.request` 替换 `site-service` 的 Mock 实现。
- 引入用户体系：登录后将收藏与反馈同步至后端账号。
- 启用 POI 导入：参考 `docs/poi_import.md` 完成后端任务，按文档约定的字段返回数据，小程序无需改动数据结构。
- 富化展示：增加图片上传、多季节天气提示、附近营地推荐等。
