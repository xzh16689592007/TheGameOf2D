import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { SpreadsheetFile, Workbook } from "@oai/artifact-tool";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, "..", "..");
const sourceJson = path.join(root, "tools", "week2_docx_content.json");
const outputDir = path.join(root, "outputs", "week2_report");
const outputPath = path.join(outputDir, "Week2_Report_Modeng.xlsx");

const data = JSON.parse(await fs.readFile(sourceJson, "utf8"));

const tables = data.tables ?? [];
const baseInfo = tables[0] ?? [];
const moduleRows = tables[1] ?? [];
const memberRows = tables[2] ?? [];
const screenshotRows = tables[3] ?? [];
const paragraphs = data.paragraphs ?? [];

const afterHeading = (heading) => {
  const idx = paragraphs.indexOf(heading);
  if (idx < 0) return [];
  const stop = new Set([
    "模块状态",
    "本周小结",
    "小组总结",
    "组员个人小结",
    "小组讨论纪要和必要截图",
    "截图",
  ]);
  const result = [];
  for (let i = idx + 1; i < paragraphs.length; i++) {
    if (stop.has(paragraphs[i])) break;
    result.push(paragraphs[i]);
  }
  return result;
};

const title = paragraphs.find((p) => p.includes("第二周项目进度总结")) ?? "《墨灯守巷》第二周项目进度总结";
const groupSummary = afterHeading("小组总结");
const meetingNotes = afterHeading("小组讨论纪要和必要截图");

const workbook = Workbook.create();

const theme = {
  dark: "#254E58",
  mid: "#5E8C7A",
  light: "#EAF3EF",
  pale: "#F7FAF8",
  border: "#CAD8D2",
  text: "#1F2933",
  accent: "#A06A3B",
};

function setWidths(sheet, widths) {
  widths.forEach((width, index) => {
    sheet.getRangeByIndexes(0, index, 1, 1).format.columnWidthPx = width;
  });
}

function styleSheetBase(sheet, lastCol, lastRow) {
  const used = sheet.getRangeByIndexes(0, 0, Math.max(lastRow, 1), Math.max(lastCol, 1));
  used.format = {
    font: { color: theme.text },
    wrapText: true,
    verticalAlignment: "top",
  };
  used.format.borders = { preset: "all", style: "thin", color: theme.border };
}

function styleTitle(sheet, range, text) {
  const r = sheet.getRange(range);
  r.merge();
  r.values = [[text]];
  r.format = {
    fill: theme.dark,
    font: { bold: true, color: "#FFFFFF", size: 16 },
    horizontalAlignment: "center",
    verticalAlignment: "middle",
  };
  r.format.rowHeightPx = 42;
}

function styleSection(sheet, range, text) {
  const r = sheet.getRange(range);
  r.merge();
  r.values = [[text]];
  r.format = {
    fill: theme.light,
    font: { bold: true, color: theme.dark, size: 12 },
    verticalAlignment: "middle",
  };
  r.format.rowHeightPx = 28;
}

function header(range) {
  range.format = {
    fill: theme.mid,
    font: { bold: true, color: "#FFFFFF" },
    horizontalAlignment: "center",
    verticalAlignment: "middle",
    wrapText: true,
  };
  range.format.rowHeightPx = 30;
}

function addTable(sheet, address, rows) {
  if (!rows.length) return;
  sheet.getRange(address).values = rows;
}

const overview = workbook.worksheets.add("周报总览");
setWidths(overview, [120, 260, 120, 260]);
styleTitle(overview, "A1:D1", "每周总结报告");
overview.getRange("A2:D2").merge();
overview.getRange("A2:D2").values = [[title]];
overview.getRange("A2:D2").format = {
  fill: theme.pale,
  font: { bold: true, color: theme.dark, size: 13 },
  horizontalAlignment: "center",
  verticalAlignment: "middle",
};
overview.getRange("A2:D2").format.rowHeightPx = 34;
overview.getRange("A4:B6").values = baseInfo.map((row) => [row[0] ?? "", row[1] ?? ""]);
overview.getRange("A4:A6").format = {
  fill: theme.light,
  font: { bold: true, color: theme.dark },
  horizontalAlignment: "center",
  verticalAlignment: "middle",
};
overview.getRange("B4:B6").format = { verticalAlignment: "middle" };
styleSection(overview, "A8:D8", "本周进度概览");
overview.getRange("A9:D13").merge();
overview.getRange("A9:D13").values = [[groupSummary[0] ?? ""]];
overview.getRange("A9:D13").format = { wrapText: true, verticalAlignment: "top" };
overview.getRange("A9:D13").format.rowHeightPx = 30;
styleSection(overview, "A15:D15", "当前风险与后续重点");
overview.getRange("A16:D21").merge();
overview.getRange("A16:D21").values = [[groupSummary[2] ?? ""]];
overview.getRange("A16:D21").format = { wrapText: true, verticalAlignment: "top" };
overview.getRange("A16:D21").format.rowHeightPx = 30;
styleSheetBase(overview, 4, 21);

const modules = workbook.worksheets.add("模块状态");
setWidths(modules, [90, 90, 160, 620]);
if (moduleRows.length) {
  modules.getRangeByIndexes(0, 0, moduleRows.length, moduleRows[0].length).values = moduleRows;
  header(modules.getRangeByIndexes(0, 0, 1, moduleRows[0].length));
  modules.freezePanes.freezeRows(1);
  modules.getRangeByIndexes(1, 0, Math.max(moduleRows.length - 1, 1), 1).format = {
    horizontalAlignment: "center",
    verticalAlignment: "middle",
  };
}
styleSheetBase(modules, 4, Math.max(moduleRows.length, 1));

const summary = workbook.worksheets.add("本周小结");
setWidths(summary, [110, 760]);
styleTitle(summary, "A1:B1", "本周小结");
summary.getRange("A3:B3").values = [["序号", "小组总结"]];
header(summary.getRange("A3:B3"));
const summaryRows = groupSummary.map((text, idx) => [String(idx + 1), text]);
if (summaryRows.length) {
  summary.getRangeByIndexes(3, 0, summaryRows.length, 2).values = summaryRows;
  summary.getRangeByIndexes(3, 0, summaryRows.length, 1).format = {
    horizontalAlignment: "center",
    verticalAlignment: "middle",
  };
}
styleSheetBase(summary, 2, Math.max(4 + summaryRows.length, 6));

const members = workbook.worksheets.add("个人小结");
setWidths(members, [120, 760]);
if (memberRows.length) {
  members.getRangeByIndexes(0, 0, memberRows.length, memberRows[0].length).values = memberRows;
  header(members.getRangeByIndexes(0, 0, 1, memberRows[0].length));
  members.freezePanes.freezeRows(1);
  members.getRangeByIndexes(1, 0, Math.max(memberRows.length - 1, 1), 1).format = {
    horizontalAlignment: "center",
    verticalAlignment: "middle",
    font: { bold: true, color: theme.dark },
  };
}
styleSheetBase(members, 2, Math.max(memberRows.length, 1));

const meeting = workbook.worksheets.add("讨论纪要与截图");
setWidths(meeting, [90, 780]);
styleTitle(meeting, "A1:B1", "小组讨论纪要和必要截图");
meeting.getRange("A3:B3").values = [["序号", "讨论纪要"]];
header(meeting.getRange("A3:B3"));
const noteRows = meetingNotes.filter((p) => p !== "截图").map((text, idx) => [String(idx + 1), text]);
if (noteRows.length) {
  meeting.getRangeByIndexes(3, 0, noteRows.length, 2).values = noteRows;
}
const screenshotStart = 5 + noteRows.length;
styleSection(meeting, `A${screenshotStart}:B${screenshotStart}`, "必要截图清单");
if (screenshotRows.length) {
  const start = screenshotStart + 1;
  meeting.getRangeByIndexes(start - 1, 0, screenshotRows.length, screenshotRows[0].length).values = screenshotRows;
  header(meeting.getRangeByIndexes(start - 1, 0, 1, screenshotRows[0].length));
}
styleSheetBase(meeting, 2, screenshotStart + screenshotRows.length);

for (const sheet of [modules, summary, members, meeting]) {
  const used = sheet.getUsedRange();
  used.format.autofitRows();
}

await fs.mkdir(outputDir, { recursive: true });

const checks = [];
checks.push(await workbook.inspect({
  kind: "table",
  range: "周报总览!A1:D21",
  include: "values,formulas",
  tableMaxRows: 20,
  tableMaxCols: 6,
}));
checks.push(await workbook.inspect({
  kind: "table",
  range: "个人小结!A1:B8",
  include: "values,formulas",
  tableMaxRows: 10,
  tableMaxCols: 4,
}));
checks.push(await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A|\\?\\?\\?\\?",
  options: { useRegex: true, maxResults: 50 },
  summary: "final error and mojibake scan",
}));

for (const [sheetName, range] of [
  ["周报总览", "A1:D21"],
  ["模块状态", `A1:D${Math.max(moduleRows.length, 1)}`],
  ["本周小结", `A1:B${Math.max(4 + summaryRows.length, 6)}`],
  ["个人小结", `A1:B${Math.max(memberRows.length, 1)}`],
  ["讨论纪要与截图", `A1:B${screenshotStart + screenshotRows.length}`],
]) {
  const blob = await workbook.render({ sheetName, range, autoCrop: "all", scale: 1, format: "png" });
  await fs.writeFile(path.join(outputDir, `${sheetName}.png`), Buffer.from(await blob.arrayBuffer()));
}

const xlsx = await SpreadsheetFile.exportXlsx(workbook);
await xlsx.save(outputPath);

console.log(JSON.stringify({
  outputPath,
  sheets: ["周报总览", "模块状态", "本周小结", "个人小结", "讨论纪要与截图"],
  checks: checks.map((c) => c.ndjson).join("\n"),
}, null, 2));
