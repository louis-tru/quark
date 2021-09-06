void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(4);
    paint.setColor(0xff4285F4);

    SkRect rect = SkRect::MakeXYWH(10, 80, 100, 160);
    // canvas->drawRect(rect, paint);

    SkRRect rrect;// = SKRRect::MakeRect(rect);
  	SkVector radii[4] = {{10,20},{10,20},{10,20},{10,20}};
  	rrect.setRectRadii(rect, radii);
    paint.setColor(0xffDB4437);
  	//paint.setStyle(SkPaint::kStroke_Style);
	//paint.setStrokeWidth(5);
    canvas->drawRRect(rrect, paint);
  
  	SkRRect rrect0, rrect1;
  	SkVector radii0[4] = {{10,20},{10,20},{10,20},{10,20}};
	SkVector radii1[4] = {{10,20},{10,20},{10,20},{10,20}};
	rrect0.setRectRadii(SkRect::MakeXYWH(50, 60, 100, 160), radii0);
  	rrect1.setRectRadii(SkRect::MakeXYWH(60, 70, 80, 140), radii1);
    paint.setColor(0xff0000ff);
	//paint.setStyle(SkPaint::kStroke_Style);
    //paint.setStrokeWidth(1);
	canvas->drawDRRect(rrect0, rrect1, paint);
  
  	// drawCircle
  	paint.setColor(0xff00ff00);
  	canvas->drawCircle(180, 40, 30, paint);
  
  	// drawArc
    paint.setColor(0xffffff00);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawArc(SkRect::MakeXYWH(80, 60, 100, 160), 0, 180, 0, paint);
  
}