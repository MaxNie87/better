/**
 * MVP 阶段的露营地点模拟数据。
 *
 * 字段说明：
 *   id              露营点唯一 ID（小程序内主键）
 *   name            名称
 *   description     简介
 *   note            备注
 *   latitude        纬度（WGS84/GCJ02 由 source 决定，MVP 默认 GCJ02 适配微信地图）
 *   longitude       经度
 *   address         展示用地址
 *   seasons         适合季节，取值：spring/summer/autumn/winter
 *   hasToilet       是否有卫生间（true/false/null=未知）
 *   isActive        营地是否仍在使用（true/false/null=未知）
 *   tags            自定义标签
 *   source          数据来源，例如 manual / poi_amap / poi_tencent
 *   externalPoiId   外部 POI ID（用于去重）
 *   updatedAt       最后更新时间（ISO 字符串）
 *   coverImage      可选封面图链接（MVP 使用占位）
 */

const CAMP_SITES = [
  {
    id: 'cs_001',
    name: '青城山后山溪边营地',
    description: '位于青城后山溪流旁，林荫覆盖，适合夏季避暑。',
    note: '需自带饮用水，附近无明显补给点。',
    latitude: 30.9182,
    longitude: 103.5012,
    address: '四川省成都市都江堰市青城后山',
    seasons: ['spring', 'summer', 'autumn'],
    hasToilet: true,
    isActive: true,
    tags: ['溪边', '林荫', '亲子'],
    source: 'manual',
    externalPoiId: null,
    updatedAt: '2026-04-12T08:00:00.000Z',
    coverImage: ''
  },
  {
    id: 'cs_002',
    name: '龙泉山观景平台营地',
    description: '可俯瞰成都平原，傍晚日落角度优秀。',
    note: '风较大，注意天幕固定。',
    latitude: 30.5821,
    longitude: 104.3099,
    address: '四川省成都市龙泉驿区龙泉山城市森林公园',
    seasons: ['spring', 'autumn', 'winter'],
    hasToilet: false,
    isActive: true,
    tags: ['观景', '日落'],
    source: 'manual',
    externalPoiId: null,
    updatedAt: '2026-03-20T10:30:00.000Z',
    coverImage: ''
  },
  {
    id: 'cs_003',
    name: '彭州丹景山林间空地',
    description: '林下空地，相对隐秘，停车需走 10 分钟。',
    note: '雨季泥泞，请避开雨后第二天。',
    latitude: 31.0345,
    longitude: 103.9421,
    address: '四川省成都市彭州市丹景山镇',
    seasons: ['spring', 'autumn'],
    hasToilet: false,
    isActive: false,
    tags: ['林下', '隐秘'],
    source: 'poi_amap',
    externalPoiId: 'amap_B0FFF000001',
    updatedAt: '2025-11-18T03:45:00.000Z',
    coverImage: ''
  },
  {
    id: 'cs_004',
    name: '简阳三岔湖湖畔营地',
    description: '湖畔大草坪营地，配套较完善，适合家庭。',
    note: '周末人流密集，建议提前到场。',
    latitude: 30.4101,
    longitude: 104.5642,
    address: '四川省成都市简阳市三岔湖',
    seasons: ['spring', 'summer', 'autumn'],
    hasToilet: true,
    isActive: true,
    tags: ['湖景', '家庭', '草坪'],
    source: 'poi_tencent',
    externalPoiId: 'tencent_4623891',
    updatedAt: '2026-04-25T12:10:00.000Z',
    coverImage: ''
  },
  {
    id: 'cs_005',
    name: '崇州鸡冠山森林营地',
    description: '高海拔森林营地，夏季凉爽。',
    note: '海拔较高，注意防寒。',
    latitude: 30.7833,
    longitude: 103.2745,
    address: '四川省成都市崇州市鸡冠山乡',
    seasons: ['summer'],
    hasToilet: true,
    isActive: true,
    tags: ['高海拔', '避暑', '森林'],
    source: 'poi_amap',
    externalPoiId: 'amap_B0FFF000045',
    updatedAt: '2026-04-02T01:15:00.000Z',
    coverImage: ''
  },
  {
    id: 'cs_006',
    name: '都江堰虹口河滩营地',
    description: '河滩开阔，适合戏水和团建。',
    note: '汛期请勿前往，注意上游放水通知。',
    latitude: 31.0921,
    longitude: 103.6310,
    address: '四川省成都市都江堰市虹口乡',
    seasons: ['summer'],
    hasToilet: null,
    isActive: null,
    tags: ['河滩', '戏水'],
    source: 'manual',
    externalPoiId: null,
    updatedAt: '2025-08-09T07:20:00.000Z',
    coverImage: ''
  }
];

module.exports = {
  CAMP_SITES
};
