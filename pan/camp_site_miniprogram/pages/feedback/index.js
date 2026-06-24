const app = getApp();
const siteService = require('../../utils/site-service.js');

function toTriState(value) {
  if (value === true) return 'true';
  if (value === false) return 'false';
  return 'unknown';
}

function fromTriState(value) {
  if (value === 'true') return true;
  if (value === 'false') return false;
  return null;
}

Page({
  data: {
    site: null,
    form: {
      hasToilet: 'unknown',
      isActive: 'unknown',
      note: ''
    }
  },

  onLoad(options) {
    this.siteId = options && options.id ? options.id : '';
    if (!this.siteId) {
      wx.showToast({ title: '缺少营地 ID', icon: 'none' });
      return;
    }
    siteService.getSiteById(this.siteId).then(site => {
      if (!site) {
        this.setData({ site: null });
        return;
      }
      this.setData({
        site: site,
        form: {
          hasToilet: toTriState(site.hasToilet),
          isActive: toTriState(site.isActive),
          note: site.note || ''
        }
      });
    });
  },

  onChangeToilet(e) {
    this.setData({ 'form.hasToilet': e.detail.value });
  },

  onChangeActive(e) {
    this.setData({ 'form.isActive': e.detail.value });
  },

  onChangeNote(e) {
    this.setData({ 'form.note': e.detail.value });
  },

  onCancel() {
    wx.navigateBack({ delta: 1 });
  },

  onSubmit() {
    if (!this.data.site) {
      return;
    }
    const payload = {
      hasToilet: fromTriState(this.data.form.hasToilet),
      isActive: fromTriState(this.data.form.isActive),
      note: this.data.form.note.trim()
    };

    wx.showLoading({ title: '提交中...', mask: true });
    siteService.submitFeedback(this.data.site.id, payload)
      .then(() => {
        const persisted = {};
        if (typeof payload.hasToilet === 'boolean') persisted.hasToilet = payload.hasToilet;
        if (typeof payload.isActive === 'boolean') persisted.isActive = payload.isActive;
        if (payload.note) persisted.note = payload.note;
        app.saveFeedback(this.data.site.id, persisted);
        wx.hideLoading();
        wx.showToast({ title: '已提交', icon: 'success' });
        setTimeout(() => wx.navigateBack({ delta: 1 }), 600);
      })
      .catch(err => {
        wx.hideLoading();
        console.warn('[feedback] 提交失败', err);
        wx.showToast({ title: '提交失败，请重试', icon: 'none' });
      });
  }
});
