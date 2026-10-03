import { createGMPlugin } from './vendor/gm-plugin-web-sdk.esm.js';
import {
  EVENT_CHANNEL,
  EVENT_STEP,
  EVENT_TIMER_DONE,
  RECIPE_CHANNEL,
  decodeEvent,
  encodeEnd,
  encodeRecipe,
} from './protocol.js';
import { detectLocale, getRecipes, getStrings } from './translations.js';

const LOCALE = detectLocale();
const S = getStrings(LOCALE);
const RECIPES = getRecipes(LOCALE);

const STORAGE_KEY = 'recipe';

const gm = createGMPlugin();

const statusEl = document.querySelector('#status');
const startButton = document.querySelector('#start-button');
const stopButton = document.querySelector('#stop-button');
const recipeSelect = document.querySelector('#recipe-select');
const stepsList = document.querySelector('#steps-list');
const sessionPanel = document.querySelector('#session-panel');
const stepStatusEl = document.querySelector('#step-status');
const noStatsEl = document.querySelector('#no-stats');

let connected = false;
let active = false;

function setStatus(text, state = '') {
  statusEl.textContent = text;
  statusEl.className = `status ${state}`.trim();
}

function applyStrings() {
  document.documentElement.lang = LOCALE;
  document.querySelector('#app-title').textContent = S.appTitle;
  document.querySelector('#pick-label').textContent = S.pickRecipe;
  startButton.textContent = S.start;
  stopButton.textContent = S.stop;
  document.querySelector('#glasses-hint').textContent = S.glassesHint;
  document.querySelector('#stat-recipes-label').textContent = S.statsRecipes;
  noStatsEl.textContent = S.noStats;
  setStatus(S.statusStarting);
}

function renderPicker() {
  recipeSelect.innerHTML = '';
  RECIPES.forEach((recipe, index) => {
    const option = document.createElement('option');
    option.value = String(index);
    option.textContent = recipe.name;
    recipeSelect.appendChild(option);
  });
  recipeSelect.selectedIndex = 0;
  renderSteps();
}

function renderSteps() {
  const recipe = RECIPES[recipeSelect.selectedIndex];
  stepsList.innerHTML = '';
  if (!recipe) return;
  recipe.steps.forEach((step, index) => {
    const li = document.createElement('li');
    const line = document.createElement('span');
    line.className = 'step-line';
    line.textContent = S.stepLine(index + 1, recipe.steps.length);
    const text = document.createElement('span');
    text.className = 'step-text';
    text.textContent = step.text;
    const timer = document.createElement('span');
    timer.className = 'step-timer';
    timer.textContent = S.stepTimer(step.minutes);
    li.appendChild(line);
    li.appendChild(text);
    li.appendChild(timer);
    stepsList.appendChild(li);
  });
}

async function loadStats() {
  const result = await gm.storage.get(STORAGE_KEY);
  return result.value || { sent: 0 };
}

async function renderStats() {
  const stats = await loadStats();
  document.querySelector('#stat-recipes').textContent = String(stats.sent);
  noStatsEl.hidden = stats.sent > 0;
}

async function addSent() {
  const stats = await loadStats();
  stats.sent += 1;
  await gm.storage.set(STORAGE_KEY, stats);
  await renderStats();
}

async function sendRecipe() {
  const recipe = RECIPES[recipeSelect.selectedIndex];
  if (!recipe) return;
  try {
    await gm.plugin.sendMessage(RECIPE_CHANNEL, encodeRecipe(recipe));
    await addSent();
    active = true;
    startButton.hidden = true;
    stopButton.hidden = false;
    sessionPanel.hidden = false;
    stepStatusEl.textContent = S.connected;
    setStatus(S.connected, 'ready');
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
}

async function stopRecipe() {
  active = false;
  try {
    await gm.plugin.sendMessage(RECIPE_CHANNEL, encodeEnd());
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
  startButton.hidden = false;
  stopButton.hidden = true;
  sessionPanel.hidden = true;
  startButton.disabled = !connected;
  setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
}

async function onEvent(decoded) {
  if (!active) return;
  if (decoded.event === EVENT_STEP) {
    stepStatusEl.textContent = S.glassStep(decoded.index + 1);
  } else if (decoded.event === EVENT_TIMER_DONE) {
    stepStatusEl.textContent = S.timerDone(decoded.index + 1);
    if (navigator.vibrate) navigator.vibrate([200, 100, 200]);
  }
}

recipeSelect.addEventListener('change', renderSteps);
startButton.addEventListener('click', () => void sendRecipe());
stopButton.addEventListener('click', () => void stopRecipe());

const offMessages = gm.plugin.onMessage((message) => {
  if (message.channel !== EVENT_CHANNEL) return;
  const decoded = decodeEvent(message.data);
  if (!decoded) return;
  void onEvent(decoded);
});

async function start() {
  try {
    await gm.ready();
    applyStrings();
    renderPicker();
    await renderStats();
    const info = await gm.device.getInfo();
    connected = Boolean(info.connected);
    setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
    startButton.disabled = !connected;
    await gm.device.subscribeEvents(['connection']);
    gm.device.onConnection((event) => {
      connected = Boolean(event.connected);
      setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
      if (!connected && active) void stopRecipe();
      startButton.disabled = !connected || active;
    });
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
}

window.addEventListener('pagehide', () => {
  offMessages();
  gm.close();
});

void start();
