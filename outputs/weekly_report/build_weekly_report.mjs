import fs from "node:fs/promises";
import path from "node:path";
import { SpreadsheetFile, Workbook } from "@oai/artifact-tool";

const outputDir = "C:/Users/26474/Documents/Unreal Projects/TheGameOf2D/outputs/weekly_report";
const outputPath = path.join(outputDir, "墨灯守巷_每周总结报告_第2周.xlsx");

const workbook = Workbook.create();
const report = workbook.worksheets.add("每周总结报告");
const evidence = workbook.worksheets.add("进度依据");

report.showGridLines = false;
evidence.showGridLines = false;

const palette = {
  ink: "#1F2937",
  muted: "#6B7280",
  line: "#D9E2EC",
  header: "#1F4E5F",
  header2: "#EAF3F5",
  warm: "#F6E7C8",
  progress: "#FFF3CD",
  done: "#D9EAD3",
  risk: "#FCE4D6",
  white: "#FFFFFF",
};

function setWidths(sheet, widths) {
  widths.forEach((width, i) => {
    sheet.getRangeByIndexes(0, i, 1, 1).format.columnWidthPx = width;
  });
}

function styleBlockTitle(range) {
  range.format = {
    fill: palette.header,
    font: { bold: true, color: palette.white, size: 12 },
    horizontalAlignment: "left",
    verticalAlignment: "center",
  };
}

function styleTable(range) {
  range.format = {
    borders: { preset: "all", style: "thin", color: palette.line },
    verticalAlignment: "top",
    wrapText: true,
    font: { color: palette.ink, size: 10 },
  };
}

function mergeValue(sheet, address, value, fill = null, bold = false) {
  const range = sheet.getRange(address);
  range.merge();
  range.values = [[value]];
  range.format = {
    fill: fill || palette.white,
    font: { bold, color: palette.ink, size: bold ? 11 : 10 },
    verticalAlignment: "top",
    wrapText: true,
    borders: { preset: "all", style: "thin", color: palette.line },
  };
  return range;
}

setWidths(report, [92, 130, 98, 120, 520]);
setWidths(evidence, [140, 130, 180, 520]);

report.getRange("A1:E1").merge();
report.getRange("A1").values = [["每周总结报告"]];
report.getRange("A1").format = {
  fill: palette.header,
  font: { bold: true, color: palette.white, size: 18 },
  horizontalAlignment: "center",
  verticalAlignment: "center",
};
report.getRange("A1:E1").format.rowHeightPx = 42;

report.getRange("A2:E4").format = {
  borders: { preset: "all", style: "thin", color: palette.line },
  font: { color: palette.ink, size: 11 },
  verticalAlignment: "center",
};
report.getRange("A2:A4").values = [["组别："], ["游戏名："], ["时间周期："]];
report.getRange("B2:E2").merge();
report.getRange("B3:E3").merge();
report.getRange("B4:E4").merge();
report.getRange("B2").values = [["第 1 组"]];
report.getRange("B3").values = [["墨灯守巷"]];
report.getRange("B4").values = [["2026-06-08 至 2026-06-12（第 2 周）"]];
report.getRange("A2:A4").format = { fill: palette.header2, font: { bold: true, color: palette.ink } };

report.getRange("A6:E6").merge();
report.getRange("A6").values = [["模块状态负责人详细说明"]];
styleBlockTitle(report.getRange("A6:E6"));

report.getRange("A7:E7").values = [["模块", "状态", "负责人", "计划对应", "详细说明"]];
report.getRange("A7:E7").format = {
  fill: palette.header2,
  font: { bold: true, color: palette.ink },
  horizontalAlignment: "center",
  verticalAlignment: "center",
  borders: { preset: "all", style: "thin", color: palette.line },
};

const moduleRows = [
  [
    "玩家移动与战斗",
    "进行中",
    "邢增浩",
    "第二周：普通攻击；后续扩展灯火弹、受击、血量",
    "本周主角更新由邢增浩负责。已在 SideScrollingCharacter 中推进武士刀攻击窗口、攻击检测、地面连招和跳跃/下落运动表现；新增 Begin/EndAttackTrace、Open/CloseComboWindow、CommitCombo、FinishGroundAttack 等动画通知类，并接入 Tomoe 角色动画资源。下一步需要继续联调受击、血量和灯火弹。"
  ],
  [
    "敌人与关卡规则",
    "进行中",
    "黄帝尧",
    "第二周：敌人生成、受击/死亡、灯笼耐久、胜负判断",
    "本周怪物更新由黄帝尧负责。仓库已有 ModengEnemy、ModengFastEnemy、ModengRangedEnemy、ModengExploderEnemy、ModengEnemySpawner 等 C++ 类；本周修复近战敌人配置，并补充新敌人视觉资源。下一步需要把波次刷怪、敌人追击/攻击、受击死亡和第一关胜负判定继续做成可演示闭环。"
  ],
  [
    "灯笼系统与场景资源",
    "进行中",
    "黄顺金",
    "第二周：灯笼耐久系统；第一关灰盒场景",
    "配合本周主角和怪物更新整理第一关灰盒及灯笼相关资源；仓库已有 ModengLantern C++ 类和街巷关卡资源目录。当前重点是确认第一关灰盒内灯笼、敌人出生点、玩家出生点和目标区域的摆放稳定。"
  ],
  [
    "UI、音效、测试与打包",
    "进行中",
    "张乐钊",
    "第三周起：基础 UI、音效、测试记录",
    "已有 ModengHUD、ModengEnemyHealthWidget、ModengResultWidget 等代码基础，可支持后续 HUD、血条、胜负界面。第二周主要配合最小玩法闭环，下一步应接入灯笼耐久、玩家血量、当前波次显示，并开始记录组内测试问题。"
  ],
  [
    "策划、文档与项目管理",
    "已结束",
    "黄帝尧",
    "第一周：立项、计划书、策划文档",
    "项目计划书与游戏策划文档已完成，明确 2.5D 横板动作守护玩法、两关流程、团队分工、六周开发计划和阶段验收标准。本周根据最新进度整理周报，并继续跟踪第二周玩法闭环目标。"
  ],
  [
    "渲染进阶与展示材料",
    "进行中",
    "李远涵",
    "第五周：自定义后处理效果",
    "策划中已确定灯笼亮度联动画面冷暖、饱和度和暗角变化的进阶视觉方向。当前处于前期准备阶段，建议先收集灯笼亮/暗状态截图和材质参考，等待核心玩法稳定后接入后处理参数。"
  ],
];
report.getRange("A8:E13").values = moduleRows;
styleTable(report.getRange("A8:E13"));
report.getRange("B8:B13").format.horizontalAlignment = "center";
report.getRange("B8:B11").format.fill = palette.progress;
report.getRange("B12").format.fill = palette.done;
report.getRange("B13").format.fill = palette.progress;
report.getRange("A8:E13").format.rowHeightPx = 88;

report.getRange("A15:E15").merge();
report.getRange("A15").values = [["本周小结"]];
styleBlockTitle(report.getRange("A15:E15"));

mergeValue(report, "A16:B18", "小组总结", palette.header2, true);
mergeValue(report, "C16:E18", "本周工作从第一周文档和基础原型推进到第二周最小玩法闭环。项目已拉取最新代码，主分支停在 d2c824f（Add montage-driven ground combo）。本周分工重点明确：邢增浩继续负责主角更新，完成玩家近战攻击窗口、地面连招、Tomoe 动画资源接入和跳跃/下落表现推进；黄帝尧继续负责怪物更新，推进敌人配置、敌人视觉资源和敌人生成/战斗相关基础。整体进度与第二周计划基本一致，但第一关完整胜负闭环、波次刷怪和 UI 展示仍需继续联调。");

const memberRows = [
  ["组员A：黄帝尧", "本周继续负责怪物更新，推进敌人配置、敌人视觉资源和敌人生成/战斗基础；同时维护项目计划、周报和进度跟踪。"],
  ["组员B：邢增浩", "本周继续负责主角更新，推进玩家战斗系统，完成/接入武士刀攻击检测窗口、地面连招、跳跃下落运动和 Tomoe 动画相关资源。"],
  ["组员C：黄顺金", "整理关卡与美术资源，配合灯笼、敌人、街巷场景资源摆放；后续继续完善第一关灰盒与场景层次。"],
  ["组员D：张乐钊", "准备 HUD、血条、结果界面等 UI 基础；后续接入玩家血量、灯笼耐久、波次信息并整理测试记录。"],
  ["组员E：李远涵", "确认后处理展示方向，准备灯笼亮度与画面冷暖变化的技术方案和对比素材。"],
];
report.getRange("A20:E20").merge();
report.getRange("A20").values = [["组员个人小结"]];
styleBlockTitle(report.getRange("A20:E20"));
report.getRange("A21:B25").values = memberRows.map(([name]) => [name, ""]);
report.getRange("C21:E25").values = memberRows.map(([, text]) => [text, "", ""]);
for (let r = 21; r <= 25; r++) {
  report.getRange(`A${r}:B${r}`).merge();
  report.getRange(`C${r}:E${r}`).merge();
}
styleTable(report.getRange("A21:E25"));
report.getRange("A21:B25").format = { fill: palette.header2, font: { bold: true, color: palette.ink } };
report.getRange("A21:E25").format.rowHeightPx = 54;

report.getRange("A27:E27").merge();
report.getRange("A27").values = [["小组讨论纪要和必要截图"]];
styleBlockTitle(report.getRange("A27:E27"));

const notes = [
  ["讨论主题", "本周围绕第二周验收目标进行同步：邢增浩负责主角攻击与连招更新，黄帝尧负责怪物生成、配置、受击/死亡等更新，其他成员配合场景、UI 和展示材料。"],
  ["关键结论", "优先保证“守灯笼、打敌人、胜负判定”的最小玩法可演示；主角连招和动画资源先稳定接入，怪物生成与攻击流程同步推进，UI 与音效在第三周集中补齐。"],
  ["问题与风险", "动画资源量较大，需确认蓝图引用和 C++ 动画通知联动稳定；灯笼耐久、敌人攻击灯笼和胜负判定需要尽快统一测试口径。"],
  ["下周计划", "完成第一关 Alpha 雏形：3 波刷怪、玩家血量、敌人追击/攻击、基础 HUD、投射物或击退效果，并形成一次组内测试记录。"],
  ["必要截图", "截图 1：主角地面连招/攻击窗口演示；截图 2：灯笼修复和灯光状态；截图 3：第一关灰盒场景与敌人生成点。可在 UE 中运行后粘贴到本区域或另附图片。"],
];
report.getRange("A28:B32").values = notes.map(([label]) => [label, ""]);
report.getRange("C28:E32").values = notes.map(([, text]) => [text, "", ""]);
for (let r = 28; r <= 32; r++) {
  report.getRange(`A${r}:B${r}`).merge();
  report.getRange(`C${r}:E${r}`).merge();
}
styleTable(report.getRange("A28:E32"));
report.getRange("A28:B32").format = { fill: palette.header2, font: { bold: true, color: palette.ink } };
report.getRange("A28:E32").format.rowHeightPx = 54;

report.getRange("A34:E38").merge();
report.getRange("A34").values = [["截图粘贴区"]];
report.getRange("A34:E38").format = {
  fill: "#F8FAFC",
  font: { bold: true, color: palette.muted, size: 12 },
  horizontalAlignment: "center",
  verticalAlignment: "center",
  borders: { preset: "all", style: "dashed", color: "#A7B7C7" },
};

evidence.getRange("A1:D1").merge();
evidence.getRange("A1").values = [["进度依据"]];
evidence.getRange("A1").format = {
  fill: palette.header,
  font: { bold: true, color: palette.white, size: 16 },
  horizontalAlignment: "center",
};
evidence.getRange("A3:D3").values = [["来源", "日期/周次", "条目", "摘要"]];
evidence.getRange("A3:D3").format = {
  fill: palette.header2,
  font: { bold: true, color: palette.ink },
  horizontalAlignment: "center",
  borders: { preset: "all", style: "thin", color: palette.line },
};
const evidenceRows = [
  ["项目计划书", "第二周", "阶段目标", "完成玩家普通攻击、敌人基础生成、敌人受击/死亡、灯笼耐久系统；搭建第一关灰盒场景，形成守灯笼、打敌人、胜负判定的最小玩法闭环。"],
  ["游戏策划文档", "核心玩法", "横板动作战斗 + 灯笼守护 + 波次闯关", "玩家移动、跳跃、近战攻击、灯火弹、修复灯笼；敌人逐波出现并攻击灯笼，保留灯笼并清除敌人则胜利。"],
  ["Git 提交", "2026-06-12", "d2c824f Add montage-driven ground combo", "新增地面连招流程、动画通知类和大量剑术动画资源，推进玩家普通攻击表现与检测。"],
  ["Git 提交", "2026-06-10", "69808ab Add Tomoe jump fall locomotion", "补充主角跳跃/下落运动资源，增强横板角色控制表现。"],
  ["Git 提交", "2026-06-10", "3b809c7 Add katana trace attack window", "加入武士刀攻击检测窗口，支撑普通攻击命中判定。"],
  ["Git 提交", "2026-06-10", "468472f Fix lantern repair interaction", "修复灯笼修复交互，贴合第二周灯笼耐久与修复目标。"],
  ["Git 提交", "2026-06-10", "6dc11a5 Make lantern light more visible", "增强灯笼灯光可见性，改善守护目标的视觉反馈。"],
  ["Git 提交", "2026-06-10", "9568979 Add new enemy visuals and lantern assets", "补充敌人与灯笼资产，支持第一关灰盒/资源替换。"],
  ["Git 提交", "2026-06-10", "1af8cdd Fix melee enemy loadouts", "修复近战敌人配置，推进敌人基础战斗表现。"],
];
evidence.getRange("A4:D12").values = evidenceRows;
styleTable(evidence.getRange("A4:D12"));
evidence.getRange("A4:D12").format.rowHeightPx = 48;

for (const sheet of [report, evidence]) {
  sheet.freezePanes.freezeRows(1);
}

await fs.mkdir(outputDir, { recursive: true });

const preview = await workbook.render({
  sheetName: "每周总结报告",
  range: "A1:E38",
  scale: 1,
  format: "png",
});
await fs.writeFile(path.join(outputDir, "weekly_report_preview.png"), new Uint8Array(await preview.arrayBuffer()));

const evidencePreview = await workbook.render({
  sheetName: "进度依据",
  range: "A1:D12",
  scale: 1,
  format: "png",
});
await fs.writeFile(path.join(outputDir, "weekly_report_evidence_preview.png"), new Uint8Array(await evidencePreview.arrayBuffer()));

const errors = await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
  options: { useRegex: true, maxResults: 50 },
  summary: "formula error scan",
});
console.log(errors.ndjson);

const inspect = await workbook.inspect({
  kind: "table",
  range: "每周总结报告!A1:E13",
  include: "values",
  tableMaxRows: 13,
  tableMaxCols: 5,
  maxChars: 5000,
});
console.log(inspect.ndjson);

const output = await SpreadsheetFile.exportXlsx(workbook);
await output.save(outputPath);
console.log(outputPath);
