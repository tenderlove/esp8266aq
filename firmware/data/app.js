const base_url = window.location.hash.substring(1);
const status_url = base_url + "/status.json";
const config_url = base_url + "/config.json";

function applyData(data) {
  for (let el of document.querySelectorAll(".js-raw-json")) {
    el.innerText = JSON.stringify(data, null, 2);
  }

  const class_mapping = {
    ".js-temperature": "temperature",
    ".js-humidity": "humidity",
    ".js-pm25": "pm2_5_standard"
  };

  getMeasurement = (name) => data.measurements.find(m => m.name === name).value;

  for (let selector in class_mapping) {
    const name = class_mapping[selector];

    const measurement = data.measurements.find(m => m.name === name);
    const value = measurement.value;
    const time_since = data.current_time - measurement.last_measured_at;

    for (let cell_el of document.querySelectorAll(selector)) {
      const value_el = cell_el.querySelector(".js-value");

      cell_el.classList.toggle("connected", time_since < 10000);
      value_el.innerText = Math.round(value);
    }
  }
}

async function updateData() {
  let response = await fetch(status_url);
  setTimeout(updateData, 5000);

  let data = await response.json();

  applyData(data);
}

async function onSubmitSettingsForm(ev) {
  ev.preventDefault();
  const form_el = ev.target;

  let config = {}
  for (let input_el of form_el.querySelectorAll("*[name]")) {
    config[input_el.name] = input_el.value;
  }
  console.log(config);

  const response = await fetch(config_url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(config)
  });
  console.log(response);
  await response.json();

  window.location.reload();

  return false;
}

async function setupForm() {
  const form_el = document.querySelector("form");

  form_el.addEventListener("submit", onSubmitSettingsForm);

  let response = await fetch(config_url);
  let config = await response.json();
  for (let input_el of form_el.querySelectorAll("*[name]")) {
    input_el.value = config[input_el.name] || "";
  }
}

window.addEventListener('load', (event) => {
  setupForm();

  setTimeout(updateData, 1);
});
