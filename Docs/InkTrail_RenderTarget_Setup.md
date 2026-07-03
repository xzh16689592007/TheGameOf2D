# Render Target 动态足迹设置说明

本功能用于实现“角色移动足迹 / 墨迹轨迹”。代码部分已经由 `AModengInkTrailPainter` 实现，编辑器里只需要制作两个材质并在关卡中放置一个 Actor。

## 1. 创建墨迹笔刷材质

在内容浏览器中新建材质：

`/Game/MoDeng/Materials/M_InkBrush`

材质设置：

- Material Domain：`User Interface`
- Blend Mode：`Translucent`

材质节点建议：

1. `TextureCoordinate`
2. 减去 `Constant2Vector(0.5, 0.5)`
3. 接 `Length`
4. 接 `OneMinus`
5. 接 `Power`，指数可设置为 `2` 到 `4`
6. 接 `Saturate`
7. 输出到 `Final Color` 和 `Opacity`

颜色可以先用白色。这个材质画到 Render Target 后，白色区域会代表“有墨迹”。

## 2. 创建地面显示材质

新建材质：

`/Game/MoDeng/Materials/M_InkOverlay`

材质设置：

- Material Domain：`Surface`
- Blend Mode：`Translucent`
- Shading Model：`Unlit`
- Two Sided：勾选

材质节点：

1. 添加 `Texture Sample Parameter2D`
2. 参数名必须设置为：`InkRT`
3. `Texture Sample` 的 `R` 通道乘以 `0.55`，接到 `Opacity`
4. `Emissive Color` 接一个深色，例如 `Constant3Vector(0.02, 0.015, 0.01)`

`InkRT` 这个名字必须一致，因为 C++ 会自动把运行时 Render Target 传给这个参数。

## 3. 在关卡中放置足迹 Actor

打开需要显示足迹的关卡，例如：

- `/Game/MoDeng/Maps/L_Level01_Street`
- `/Game/MoDeng/Maps/L_Level02_BridgeMarket`

在放置 Actor 面板中搜索：

`ModengInkTrailPainter`

拖入关卡。

## 4. 设置 Actor 参数

选中 `ModengInkTrailPainter`，在 Details 面板设置：

- `Ink Overlay Material`：选择 `M_InkOverlay`
- `Ink Brush Material`：选择 `M_InkBrush`
- `World Coverage`：覆盖足迹区域，建议先设置为 `X=6000, Y=1200`
- `Render Target Size`：建议 `1024`
- `Footprint Radius`：建议 `24` 到 `36`
- `Footprint Spacing`：建议 `45` 到 `65`
- `Foot Side Offset`：建议 `8` 到 `14`

把 Actor 放到角色主要移动区域中心，Z 轴略高于地面，例如比地面高 `1` 到 `3`，避免和地面闪烁。

## 5. 测试

运行游戏后，角色移动时会自动在 Actor 覆盖范围内留下足迹墨迹。

如果看不到：

- 确认 `Ink Overlay Material` 已设置
- 确认 `Ink Brush Material` 已设置
- 确认 Actor 的 Plane 覆盖了角色移动区域
- 确认 Plane 没在地面下面
- 确认 `M_InkOverlay` 里的参数名是 `InkRT`
- 确认角色移动速度超过 `Min Speed To Paint`

## 6. 文档可写说明

可在报告中描述为：

本项目实现了基于 Render Target 的动态墨迹足迹系统。系统将玩家世界坐标映射到地面覆盖平面的纹理空间，使用 C++ Canvas 在运行时向 Render Target 绘制墨迹笔刷，再由地面透明材质采样该 Render Target 显示实时生成的足迹轨迹。该方案使用离屏渲染到纹理技术，实现了随角色移动动态生成的环境表现效果。
