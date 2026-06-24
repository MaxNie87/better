const app = getApp();
const siteService = require('../../utils/site-service.js');
const format = require('../../utils/format.js');

Page({
  data: {
    site: null,
    updatedAtText: ''
  },

  onLoad(options) {
    this.siteId = options && options.id ? options.id : '';
    if (!this.siteId) {
      wx.showToast({ title: '缺少营地 ID', icon: 'none' });
      return;
    }
    this.refresh();
  },

  onShow() {
    if (this.siteId) {
      this.refresh();
    }
  },

  refresh() {
    return siteService.getSiteById(this.siteId).then(site => {
      if (!site) {
        this.setData({ site: null, updatedAtText: '' });
        return;
      }
      this.setData({
        site: site,
        updatedAtText: format.formatDateTime(site.updatedAt) || '未知'
      });
    });
  },

  onToggleFavorite() {
    if (!this.data.site) {
      return;
    }
    const isFav = app.toggleFavorite(this.data.site.id);
    wx.showToast({ title: isFav ? '已收藏' : '已取消', icon: 'none' });
    this.refresh();
  },

  onLocateOnMap() {
    if (!this.data.site) {
      return;
    }
    wx.switchTab({
      url: '/pages/map/index',
      success: () => {
        const pages = getCurrentPages();
        const mapPage = pages[pages.length - 1];
        if (mapPage && mapPage.focusSite) {
          mapPage.focusSite(this.data.site.id);
        }
      }
    });
  },

  onGoFeedback() {
    if (!this.data.site) {
      return;
    }
    wx.navigateTo({ url: '/pages/feedback/index?id=' + this.data.site.id });
  }
});
