package demo_server

type Point struct {
	x1 float32
	y1 float32
	x2 float32
	y2 float32
}

func (point *Point) getX1() float32 {
	return point.x1
}

func (point *Point) getY1() float32 {
	return point.y1
}

func (point *Point) getX2() float32 {
	return point.x2
}

func (point *Point) getY2() float32 {
	return point.y2
}
