const siteService = require('./utils/site-service.js');

App({
  globalData: {
    userLocation: null,
    favoriteIds: [],
    feedbackOverrides: {},
    defaultMapCenter: {
      latitude: 30.5728,
      longitude: 104.0668
    }
  },

  onLaunch() {
    try {
      const favoriteIds = wx.getStorageSync('favoriteIds') || [];
      const feedbackOverrides = wx.getStorageSync('feedbackOverrides') || {};
      this.globalData.favoriteIds = Array.isArray(favoriteIds) ? favoriteIds : [];
      this.globalData.feedbackOverrides = feedbackOverrides && typeof feedbackOverrides === 'object'
        ? feedbackOverrides
        : {};
    } catch (err) {
      console.warn('[app] 读取本地缓存失败', err);
    }

    siteService.hydrate({
      favoriteIds: this.globalData.favoriteIds,
      feedbackOverrides: this.globalData.feedbackOverrides
    });
  },

  setUserLocation(location) {
    this.globalData.userLocation = location;
  },

  toggleFavorite(siteId) {
    const list = new Set(this.globalData.favoriteIds);
    if (list.has(siteId)) {
      list.delete(siteId);
    } else {
      list.add(siteId);
    }
    this.globalData.favoriteIds = Array.from(list);
    siteService.setFavorites(this.globalData.favoriteIds);
    try {
      wx.setStorageSync('favoriteIds', this.globalData.favoriteIds);
    } catch (err) {
      console.warn('[app] 写入收藏失败', err);
    }
    return list.has(siteId);
  },

  isFavorite(siteId) {
    return this.globalData.favoriteIds.indexOf(siteId) >= 0;
  },

  saveFeedback(siteId, payload) {
    const overrides = Object.assign({}, this.globalData.feedbackOverrides);
    overrides[siteId] = Object.assign({}, overrides[siteId] || {}, payload, {
      updatedAt: new Date().toISOString()
    });
    this.globalData.feedbackOverrides = overrides;
    siteService.setFeedbackOverrides(overrides);
    try {
      wx.setStorageSync('feedbackOverrides', overrides);
    } catch (err) {
      console.warn('[app] 写入反馈失败', err);
    }
  }
});
