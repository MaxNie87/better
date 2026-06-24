露营地点管理微信小程序 MVP 计划

技术选择





使用原生微信小程序创建项目目录：camp_site_miniprogram。



地图页优先使用微信小程序内置 map 组件，定位和地点选择能力接入微信/腾讯位置服务。



第一版小程序通过模拟 API 或本地 JSON 数据驱动页面，后续替换为真实后端接口。



POI 导入功能放在后端任务中实现，优先使用合规地图/位置服务 API，不在小程序端直接批量抓取第三方网站页面。

MVP 功能范围





地图首页：展示露营地点 Marker，支持定位到当前位置。



营地点标记：点击 Marker 打开地点卡片和详情页。



地点详情：保存名称、备注、经纬度、适合季节、是否收藏、是否有卫生间、是否活跃等基础信息。



地点列表：以列表方式浏览已保存露营点，点击后跳转地图并定位到该点。



筛选能力：支持筛选“有卫生间”“活跃营地”“已收藏”。



用户补充：提供纠错/补充入口，让用户提交卫生间、活跃状态、备注等信息，MVP 阶段可先本地模拟提交。



POI 导入预留：设计后端导入任务的数据结构、字段映射和去重规则，MVP 阶段用静态数据或 Mock API 模拟后端结果。

主要文件结构





camp_site_miniprogram/project.config.json：微信开发者工具项目配置。



camp_site_miniprogram/app.json：页面路由、窗口配置、权限声明。



camp_site_miniprogram/app.js：小程序入口和全局状态。



camp_site_miniprogram/pages/map/index.wxml：地图首页结构。



camp_site_miniprogram/pages/map/index.js：地图定位、Marker 点击和跳转逻辑。



camp_site_miniprogram/pages/sites/index.wxml：露营地点列表结构。



camp_site_miniprogram/pages/sites/index.js：列表加载、筛选和跳转逻辑。



camp_site_miniprogram/pages/site-detail/index.wxml：地点详情页结构。



camp_site_miniprogram/pages/site-detail/index.js：详情加载、收藏和纠错入口。



camp_site_miniprogram/pages/feedback/index.wxml：地点信息补充/纠错表单。



camp_site_miniprogram/utils/site-service.js：露营地点数据读取、筛选和 Mock API 封装。



camp_site_miniprogram/data/camp-sites.js：MVP 阶段模拟露营地点数据。



camp_site_miniprogram/docs/poi_import.md：记录 POI 导入任务、字段映射、去重策略和后端接口草案。

实现步骤





初始化原生微信小程序项目并整理基础目录结构。



配置 app.json 页面路由、地图定位权限说明和基础导航样式。



定义营地点数据模型，包含是否有卫生间、是否活跃、数据来源、外部 POI ID、最后更新时间等字段。



创建 Mock 数据和 site-service，模拟后端地点列表、详情、筛选、收藏和纠错提交。



设计后端 POI 导入任务：关键词/分类搜索、行政区或经纬度范围获取、字段映射、去重和活跃状态更新。



实现地图页：当前位置、Marker 展示、地点卡片和详情跳转。



实现地点列表页：筛选“有卫生间”“活跃营地”“已收藏”，点击后跳转详情或地图定位。



实现地点详情页：展示基础信息、设施信息、活跃状态、来源和更新时间。



实现纠错/补充表单：提交卫生间、活跃状态、备注等用户反馈，MVP 阶段写入本地模拟状态。



运行小程序基础检查，记录微信开发者工具、AppID、位置服务 Key 等前置条件。

需要你准备





微信小程序 AppID。没有正式 AppID 时可以先用测试号或游客模式做本地开发。



微信开发者工具。



腾讯位置服务 Key 或小程序位置服务相关配置。MVP 可以先使用微信 map 组件和模拟数据开发。



后端导入任务需要的位置服务 API Key。具体用腾讯位置服务还是高德 Web 服务，需要在实现后端前结合授权条款再确认。

