const app = getApp();
const siteService = require('../../utils/site-service.js');

Page({
  data: {
    sites: [],
    filters: {
      hasToilet: false,
      isActive: false,
      favoriteOnly: false,
      keyword: ''
    }
  },

  onShow() {
    this.refresh();
  },

  onPullDownRefresh() {
    this.refresh().then(() => wx.stopPullDownRefresh());
  },

  refresh() {
    return siteService.listSites(this.data.filters).then(sites => {
      this.setData({ sites: sites });
      return sites;
    });
  },

  onToggleFilter(e) {
    const key = e.currentTarget.dataset.key;
    if (!key) {
      return;
    }
    const filters = Object.assign({}, this.data.filters);
    filters[key] = !filters[key];
    this.setData({ filters: filters }, () => this.refresh());
  },

  onKeywordInput(e) {
    const filters = Object.assign({}, this.data.filters, { keyword: e.detail.value });
    this.setData({ filters: filters }, () => this.refresh());
  },

  onReset() {
    this.setData({
      filters: { hasToilet: false, isActive: false, favoriteOnly: false, keyword: '' }
    }, () => this.refresh());
  },

  onTapItem(e) {
    const id = e.currentTarget.dataset.id;
    if (!id) {
      return;
    }
    wx.navigateTo({ url: '/pages/site-detail/index?id=' + id });
  },

  onGoDetail(e) {
    const id = e.currentTarget.dataset.id;
    if (!id) {
      return;
    }
    wx.navigateTo({ url: '/pages/site-detail/index?id=' + id });
  },

  onLocateOnMap(e) {
    const id = e.currentTarget.dataset.id;
    if (!id) {
      return;
    }
    wx.switchTab({
      url: '/pages/map/index',
      success: () => {
        const pages = getCurrentPages();
        const mapPage = pages[pages.length - 1];
        if (mapPage && mapPage.focusSite) {
          mapPage.focusSite(id);
        }
      }
    });
  },

  onToggleFavorite(e) {
    const id = e.currentTarget.dataset.id;
    if (!id) {
      return;
    }
    const isFav = app.toggleFavorite(id);
    wx.showToast({ title: isFav ? '已收藏' : '已取消', icon: 'none' });
    this.refresh();
  }
});
