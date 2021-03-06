// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:test/test.dart';
import 'package:ui/ui.dart';
import 'package:web_engine_tester/golden_tester.dart';

void main() {
  test('screenshot test reports success', () async {
    html.document.body.innerHtml = 'Hello world!';
    await matchGoldenFile('smoke_test.png', region: Rect.fromLTWH(0, 0, 320, 200));
  });
}
