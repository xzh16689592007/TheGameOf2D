import { Workbook } from "@oai/artifact-tool";

const workbook = Workbook.create();
workbook.worksheets.add("Sheet1").getRange("A1").values = [["test"]];
console.log(workbook.help("workbook.render", { include: "index,examples,notes", maxChars: 4000 }).ndjson);
