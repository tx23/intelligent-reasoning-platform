package demo_server

import (
	"fmt"
	"github.com/fogleman/gg"
	"image"
	"irs/utils"
	"time"
)

type Draw struct{}

func (draw *Draw) drawRec(path, deviceId, fileName string, point []Point) string {
	//加载图片
	var img image.Image
	var err error
	if path[len(path)-3:] == "jpg" {
		img, err = gg.LoadJPG(path)
	}else if path[len(path)-3: ] == "png" {
		img, err = gg.LoadPNG(path)
	}
	if err != nil {
		fmt.Println("load image err: ", err)
	}
	fmt.Println("Load image successfully!")

	size := img.Bounds().Size()
	width := size.X
	height := size.Y

	dc := gg.NewContext(width, height)
	dc.DrawImage(img, 0, 0)

	dc.SetLineWidth(2)
	dc.SetRGB(1, 0, 0)
	for i := 0; i < len(point); i++ {
		dc.DrawRectangle(float64(point[i].getX1()), float64(point[i].getY1()), float64(point[i].getX2())-float64(point[i].getX1()), float64(point[i].getY2())-float64(point[i].getY1()))
	}
	dc.Stroke()

	currentTime := time.Now().Format("2006-01-02_15:04:05")
	imgName := deviceId + "_" + fileName[:len(fileName)-4] + "_" + currentTime + ".png"
	_ = dc.SavePNG(utils.PathObj.RecordPath + imgName)

	return imgName
}
