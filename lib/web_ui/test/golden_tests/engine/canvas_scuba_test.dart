// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';
import 'package:test/test.dart';

import 'package:web_engine_tester/golden_tester.dart';

void main() async {
  final Rect region = Rect.fromLTWH(8, 8, 500, 100); // Compensate for old scuba tester padding

  BitmapCanvas canvas;

  setUp(() {
    html.document.body.style.transform = 'translate(10px, 10px)';
  });

  tearDown(() {
    html.document.body.style.transform = 'none';
    canvas.rootElement.remove();
  });

  /// Draws several lines, some aligned precisely with the pixel grid, and some
  /// that are offset by 0.5 vertically or horizontally.
  ///
  /// The produced picture stresses the antialiasing generated by the browser
  /// when positioning and rasterizing `<canvas>` tags. Aliasing artifacts can
  /// be seen depending on pixel alignment and whether antialiasing happens
  /// before or after rasterization.
  void drawMisalignedLines(BitmapCanvas canvas) {
    final PaintData linePaint = (Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 1)
        .webOnlyPaintData;

    final PaintData fillPaint =
        (Paint()..style = PaintingStyle.fill).webOnlyPaintData;

    canvas.drawRect(
      const Rect.fromLTWH(0, 0, 40, 40),
      linePaint,
    );

    canvas.drawLine(
      const Offset(10, 0),
      const Offset(10, 40),
      linePaint,
    );

    canvas.drawLine(
      const Offset(20.5, 0),
      const Offset(20, 40),
      linePaint,
    );

    canvas.drawCircle(const Offset(30, 10), 3, fillPaint);
    canvas.drawCircle(const Offset(30.5, 30), 3, fillPaint);
  }

  test('renders pixels that are not aligned inside the canvas', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 60, 60));

    drawMisalignedLines(canvas);

    html.document.body.append(canvas.rootElement);

    await matchGoldenFile('engine/misaligned_pixels_in_canvas_test.png', region: region);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('compensates for misalignment of the canvas', () async {
    // Notice the 0.5 offset in the bounds rectangle. It's what causes the
    // misalignment of canvas relative to the pixel grid. BitmapCanvas will
    // shift its position back to 0.0 and at the same time it will it will
    // compensate by shifting the contents of the canvas in the opposite
    // direction.
    canvas = BitmapCanvas(const Rect.fromLTWH(0.5, 0.5, 60, 60));

    drawMisalignedLines(canvas);

    html.document.body.append(canvas.rootElement);

    await matchGoldenFile('engine/misaligned_canvas_test.png', region: region);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('fill the whole canvas with color even when transformed', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 50, 50));

    canvas.translate(25, 25);
    canvas.drawColor(const Color.fromRGBO(0, 255, 0, 1.0), BlendMode.src);

    html.document.body.append(canvas.rootElement);

    await matchGoldenFile('engine/bitmap_canvas_fills_color_when_transformed.png', region: region);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('fill the whole canvas with paint even when transformed', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 50, 50));

    canvas.translate(25, 25);
    canvas.drawPaint(PaintData()
      ..color = const Color.fromRGBO(0, 255, 0, 1.0)
      ..style = PaintingStyle.fill);

    html.document.body.append(canvas.rootElement);

    await matchGoldenFile('engine/bitmap_canvas_fills_paint_when_transformed.png', region: region);
  }, timeout: const Timeout(Duration(seconds: 10)));
}
