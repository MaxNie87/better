/**
 * site-service 模块
 *
 * MVP 阶段使用本地 Mock 数据 + 内存覆盖（来自用户反馈/收藏）模拟后端能力。
 * 后续接入真实接口时，仅需替换 fetch* 系列方法的实现，保持对外签名不变。
 */

const { CAMP_SITES } = require('../data/camp-sites.js');

const SEASON_LABELS = {
  spring: '春',
  summer: '夏',
  autumn: '秋',
  winter: '冬'
};

const SOURCE_LABELS = {
  manual: '人工录入',
  poi_amap: '高德 POI 导入',
  poi_tencent: '腾讯位置服务导入'
};

const state = {
  favoriteIds: [],
  feedbackOverrides: {}
};

function clone(value) {
  if (value === null || typeof value !== 'object') {
    return value;
  }
  if (Array.isArray(value)) {
    return value.map(clone);
  }
  const out = {};
  Object.keys(value).forEach(function (key) {
    out[key] = clone(value[key]);
  });
  return out;
}

function applyOverrides(site) {
  const override = state.feedbackOverrides[site.id];
  const next = clone(site);
  next.isFavorite = state.favoriteIds.indexOf(site.id) >= 0;
  next.seasonLabels = (next.seasons || []).map(function (s) {
    return SEASON_LABELS[s] || s;
  });
  next.sourceLabel = SOURCE_LABELS[next.source] || next.source || '未知来源';

  if (override) {
    if (typeof override.hasToilet === 'boolean') {
      next.hasToilet = override.hasToilet;
    }
    if (typeof override.isActive === 'boolean') {
      next.isActive = override.isActive;
    }
    if (override.note) {
      next.note = override.note;
    }
    if (override.updatedAt) {
      next.updatedAt = override.updatedAt;
    }
    next.hasUserFeedback = true;
  } else {
    next.hasUserFeedback = false;
  }

  return next;
}

function matchFilters(site, filters) {
  if (!filters) {
    return true;
  }
  if (filters.hasToilet && site.hasToilet !== true) {
    return false;
  }
  if (filters.isActive && site.isActive !== true) {
    return false;
  }
  if (filters.favoriteOnly && !site.isFavorite) {
    return false;
  }
  if (filters.season && (site.seasons || []).indexOf(filters.season) < 0) {
    return false;
  }
  if (filters.keyword) {
    const keyword = String(filters.keyword).toLowerCase();
    const hay = [site.name, site.address, site.note, (site.tags || []).join(',')]
      .filter(Boolean)
      .join(' ')
      .toLowerCase();
    if (hay.indexOf(keyword) < 0) {
      return false;
    }
  }
  return true;
}

function simulate(value, delay) {
  return new Promise(function (resolve) {
    setTimeout(function () {
      resolve(clone(value));
    }, typeof delay === 'number' ? delay : 80);
  });
}

function hydrate(payload) {
  if (!payload) {
    return;
  }
  if (Array.isArray(payload.favoriteIds)) {
    state.favoriteIds = payload.favoriteIds.slice();
  }
  if (payload.feedbackOverrides && typeof payload.feedbackOverrides === 'object') {
    state.feedbackOverrides = clone(payload.feedbackOverrides);
  }
}

function setFavorites(ids) {
  state.favoriteIds = Array.isArray(ids) ? ids.slice() : [];
}

function setFeedbackOverrides(overrides) {
  state.feedbackOverrides = overrides && typeof overrides === 'object' ? clone(overrides) : {};
}

function listSites(filters) {
  const list = CAMP_SITES.map(applyOverrides).filter(function (site) {
    return matchFilters(site, filters);
  });
  return simulate(list);
}

function listSitesSync(filters) {
  return CAMP_SITES.map(applyOverrides).filter(function (site) {
    return matchFilters(site, filters);
  });
}

function getSiteById(id) {
  const found = CAMP_SITES.find(function (site) {
    return site.id === id;
  });
  if (!found) {
    return simulate(null);
  }
  return simulate(applyOverrides(found));
}

function submitFeedback(siteId, payload) {
  return new Promise(function (resolve, reject) {
    const exists = CAMP_SITES.some(function (site) {
      return site.id === siteId;
    });
    if (!exists) {
      reject(new Error('SITE_NOT_FOUND'));
      return;
    }
    setTimeout(function () {
      resolve({
        ok: true,
        siteId: siteId,
        payload: clone(payload),
        receivedAt: new Date().toISOString()
      });
    }, 200);
  });
}

module.exports = {
  hydrate: hydrate,
  setFavorites: setFavorites,
  setFeedbackOverrides: setFeedbackOverrides,
  listSites: listSites,
  listSitesSync: listSitesSync,
  getSiteById: getSiteById,
  submitFeedback: submitFeedback,
  SEASON_LABELS: SEASON_LABELS,
  SOURCE_LABELS: SOURCE_LABELS
};
