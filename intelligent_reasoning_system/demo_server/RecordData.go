package demo_server

type RecordData struct {
	Time       string `json:"time" gorm:"column:time"`
	DeviceName string `json:"devicename" gorm:"column:devicename"`
	ImgName    string `json:"imgname" gorm:"column:imgname"`
}
