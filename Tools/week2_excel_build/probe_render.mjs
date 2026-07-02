import { Workbook } from "@oai/artifact-tool";

const workbook = Workbook.create();
const sheet = workbook.worksheets.add("Sheet1");
sheet.getRange("A1:B2").values = [["A", "B"], ["test", "测试"]];
const png = await workbook.render({ sheetName: "Sheet1", range: "A1:B2", format: "png" });
console.log({
  type: typeof png,
  constructor: png?.constructor?.name,
  isBuffer: Buffer.isBuffer(png),
  keys: png && typeof png === "object" ? Object.keys(png) : [],
  byteLength: png?.byteLength,
  length: png?.length,
});
