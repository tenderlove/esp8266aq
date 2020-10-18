const base_url = "";

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
  const url = base_url + "/status.json";
  let response = await fetch(url);
  let data = await response.json();

  applyData(data);
}

window.addEventListener('load', (event) => {
  console.log('page is fully loaded');

  setInterval(updateData, 1000);
});
