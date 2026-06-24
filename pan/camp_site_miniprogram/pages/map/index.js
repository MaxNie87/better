const app = getApp();
const siteService = require('../../utils/site-service.js');

function buildMarker(site) {
  const tags = [];
  if (site.hasToilet === true) {
    tags.push('卫生间');
  }
  if (site.isActive === false) {
    tags.push('停用');
  }
  if (site.isFavorite) {
    tags.push('收藏');
  }
  return {
    id: hashId(site.id),
    siteId: site.id,
    latitude: site.latitude,
    longitude: site.longitude,
    title: site.name,
    iconPath: '',
    width: 32,
    height: 32,
    callout: {
      content: site.name + (tags.length ? '（' + tags.join('·') + '）' : ''),
      color: '#1f2d24',
      fontSize: 12,
      borderRadius: 8,
      borderWidth: 1,
      borderColor: '#2f6f4e',
      bgColor: '#ffffff',
      padding: 6,
      display: 'BYCLICK',
      textAlign: 'center'
    }
  };
}

function hashId(stringId) {
  let h = 0;
  for (let i = 0; i < stringId.length; i++) {
    h = (h * 31 + stringId.charCodeAt(i)) | 0;
  }
  return Math.abs(h);
}

Page({
  data: {
    center: { latitude: 30.5728, longitude: 104.0668 },
    scale: 11,
    markers: [],
    sites: [],
    activeSite: null
  },

  onLoad(options) {
    const target = options && options.siteId ? options.siteId : '';
    this.refreshSites().then(() => {
      if (target) {
        this.focusSite(target);
      }
    });
  },

  onShow() {
    this.refreshSites().then(() => {
      if (this.data.activeSite) {
        const refreshed = this.data.sites.find(s => s.id === this.data.activeSite.id);
        if (refreshed) {
          this.setData({ activeSite: refreshed });
        }
      }
    });
  },

  refreshSites() {
    return siteService.listSites().then(sites => {
      const markers = sites.map(buildMarker);
      this.setData({ sites: sites, markers: markers });
      return sites;
    });
  },

  focusSite(siteId) {
    const site = this.data.sites.find(s => s.id === siteId);
    if (!site) {
      return;
    }
    this.setData({
      activeSite: site,
      center: { latitude: site.latitude, longitude: site.longitude },
      scale: 14
    });
  },

  onMarkerTap(e) {
    const tapped = this.data.markers.find(m => m.id === e.markerId);
    if (!tapped) {
      return;
    }
    const site = this.data.sites.find(s => s.id === tapped.siteId);
    if (!site) {
      return;
    }
    this.setData({ activeSite: site });
  },

  onCloseCard() {
    this.setData({ activeSite: null });
  },

  onRegionChange() {},

  onLocate() {
    wx.getLocation({
      type: 'gcj02',
      success: res => {
        app.setUserLocation({ latitude: res.latitude, longitude: res.longitude });
        this.setData({
          center: { latitude: res.latitude, longitude: res.longitude },
          scale: 13
        });
      },
      fail: err => {
        console.warn('[map] 定位失败', err);
        wx.showToast({ title: '定位失败，请检查授权', icon: 'none' });
      }
    });
  },

  onGoList() {
    wx.switchTab({ url: '/pages/sites/index' });
  },

  onGoDetail() {
    if (!this.data.activeSite) {
      return;
    }
    wx.navigateTo({ url: '/pages/site-detail/index?id=' + this.data.activeSite.id });
  },

  onToggleFavorite() {
    if (!this.data.activeSite) {
      return;
    }
    const isFav = app.toggleFavorite(this.data.activeSite.id);
    wx.showToast({ title: isFav ? '已收藏' : '已取消', icon: 'none' });
    this.refreshSites().then(() => {
      const refreshed = this.data.sites.find(s => s.id === this.data.activeSite.id);
      if (refreshed) {
        this.setData({ activeSite: refreshed });
      }
    });
  }
});
