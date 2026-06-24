function pad(num) {
  return num < 10 ? '0' + num : '' + num;
}

function formatDateTime(value) {
  if (!value) {
    return '';
  }
  const date = value instanceof Date ? value : new Date(value);
  if (isNaN(date.getTime())) {
    return '';
  }
  return date.getFullYear() + '-' + pad(date.getMonth() + 1) + '-' + pad(date.getDate())
    + ' ' + pad(date.getHours()) + ':' + pad(date.getMinutes());
}

function formatDate(value) {
  if (!value) {
    return '';
  }
  const date = value instanceof Date ? value : new Date(value);
  if (isNaN(date.getTime())) {
    return '';
  }
  return date.getFullYear() + '-' + pad(date.getMonth() + 1) + '-' + pad(date.getDate());
}

function describeBool(value, yes, no, unknown) {
  if (value === true) {
    return yes;
  }
  if (value === false) {
    return no;
  }
  return unknown;
}

module.exports = {
  formatDate: formatDate,
  formatDateTime: formatDateTime,
  describeBool: describeBool
};
