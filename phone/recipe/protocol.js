// Binary wire protocol shared with GlassSDK/examples/recipe/recipe.c.
// Keep both sides in sync if you change any layout or constant here.

export const RECIPE_CHANNEL = 0x5243; // 'RC', phone -> glasses
export const EVENT_CHANNEL = 0x5241; // 'RA', glasses -> phone
export const PROTOCOL_VERSION = 1;

export const COMMAND_RECIPE = 1;
export const COMMAND_END = 2;

export const EVENT_TIMER_DONE = 1;
export const EVENT_STEP = 2;

export const MAX_STEPS = 20;
export const MAX_STEP_BYTES = 160;
export const MAX_RECIPE_NAME_BYTES = 40;

const encoder = new TextEncoder();
const decoder = new TextDecoder();

function toBoundedUtf8(text, maxBytes) {
  let bytes = encoder.encode(text);
  while (bytes.length > maxBytes) {
    bytes = encoder.encode([...decoder.decode(bytes)].slice(0, -1).join(''));
  }
  return bytes;
}

// [version, COMMAND_RECIPE, name_len, name...,
//  count_lo, count_hi,
//  (minutes_lo, minutes_hi, step_len, step...)*]
export function encodeRecipe({ name, steps }) {
  const nameBytes = toBoundedUtf8(name, MAX_RECIPE_NAME_BYTES);
  const limited = steps.slice(0, MAX_STEPS);
  const stepBytes = limited.map((step) => ({
    minutes: Math.max(0, Math.min(65535, Math.round(step.minutes || 0))),
    bytes: toBoundedUtf8(step.text, MAX_STEP_BYTES),
  }));
  const body = stepBytes.reduce((sum, step) => sum + 3 + step.bytes.length, 0);
  const payload = new Uint8Array(5 + nameBytes.length + body);
  let offset = 0;
  payload[offset++] = PROTOCOL_VERSION;
  payload[offset++] = COMMAND_RECIPE;
  payload[offset++] = nameBytes.length;
  payload.set(nameBytes, offset);
  offset += nameBytes.length;
  payload[offset++] = limited.length & 0xff;
  payload[offset++] = (limited.length >> 8) & 0xff;
  stepBytes.forEach((step) => {
    payload[offset++] = step.minutes & 0xff;
    payload[offset++] = (step.minutes >> 8) & 0xff;
    payload[offset++] = step.bytes.length;
    payload.set(step.bytes, offset);
    offset += step.bytes.length;
  });
  return payload;
}

export function encodeEnd() {
  return new Uint8Array([PROTOCOL_VERSION, COMMAND_END, 0, 0, 0]);
}

// [version, event, index_lo, index_hi]
export function decodeEvent(data) {
  if (!data || data.length < 4 || data[0] !== PROTOCOL_VERSION) return null;
  const event = data[1];
  if (event !== EVENT_TIMER_DONE && event !== EVENT_STEP) return null;
  return { event, index: data[2] | (data[3] << 8) };
}
