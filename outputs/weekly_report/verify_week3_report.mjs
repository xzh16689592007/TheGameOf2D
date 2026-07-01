import fs from "node:fs/promises";
import path from "node:path";
import { FileBlob, SpreadsheetFile } from "@oai/artifact-tool";

const outputDir = "C:/Users/26474/Documents/Unreal Projects/TheGameOf2D/outputs/weekly_report";
const xlsxPath = "C:/Users/26474/OneDrive/文档/1组-第3周总结报告.xlsx";

const input = await FileBlob.load(xlsxPath);
const workbook = await SpreadsheetFile.importXlsx(input);

const errors = await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
  options: { useRegex: true, maxResults: 50 },
  summary: "formula error scan",
});
console.log(errors.ndjson);

for (const sheetName of ["周报总览", "模块状态", "本周小结", "个人小结", "讨论纪要与截图"]) {
  const preview = await workbook.render({
    sheetName,
    autoCrop: "all",
    scale: 1,
    format: "png",
  });
  await fs.writeFile(
    path.join(outputDir, `week3_${sheetName}.png`),
    new Uint8Array(await preview.arrayBuffer()),
  );
}

console.log("rendered");
