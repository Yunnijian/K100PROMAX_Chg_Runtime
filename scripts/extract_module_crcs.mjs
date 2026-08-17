import { readFileSync, writeFileSync } from 'node:fs';

const [input, output, ...wanted] = process.argv.slice(2);
if (!input || !output || wanted.length === 0) {
  throw new Error('usage: node extract_module_crcs.mjs INPUT.ko OUTPUT.Module.symvers symbol...');
}

const data = readFileSync(input);
if (data.readUInt32LE(0) !== 0x464c457f || data[4] !== 2 || data[5] !== 1) {
  throw new Error('input must be ELF64 little-endian');
}

const u16 = (offset) => data.readUInt16LE(offset);
const u32 = (offset) => data.readUInt32LE(offset);
const u64 = (offset) => Number(data.readBigUInt64LE(offset));
const sectionOffset = u64(0x28);
const sectionSize = u16(0x3a);
const sectionCount = u16(0x3c);
const stringIndex = u16(0x3e);
const section = (index) => {
  const offset = sectionOffset + index * sectionSize;
  return { name: u32(offset), offset: u64(offset + 24), size: u64(offset + 32) };
};
const strings = section(stringIndex);
const nameAt = (offset) => {
  const start = strings.offset + offset;
  return data.toString('utf8', start, data.indexOf(0, start));
};

let versions;
for (let index = 0; index < sectionCount; index += 1) {
  const candidate = section(index);
  if (nameAt(candidate.name) === '__versions') versions = candidate;
}
if (!versions) throw new Error('input has no __versions section');

const found = new Map();
for (let offset = versions.offset; offset < versions.offset + versions.size; offset += 64) {
  const end = data.indexOf(0, offset + 8);
  const symbol = data.toString('utf8', offset + 8, end);
  if (wanted.includes(symbol)) {
    const crc = `0x${data.readBigUInt64LE(offset).toString(16).slice(-8).padStart(8, '0')}`;
    found.set(symbol, crc);
  }
}

const missing = wanted.filter((symbol) => !found.has(symbol));
if (missing.length) throw new Error(`missing symbols: ${missing.join(', ')}`);
writeFileSync(output, `${wanted.map((symbol) => `${found.get(symbol)}\t${symbol}\tvmlinux\tEXPORT_SYMBOL`).join('\n')}\n`);
