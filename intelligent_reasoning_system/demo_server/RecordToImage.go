package demo_server

type RecordToImage struct {
	Id        string `gorm:"column:Id"`
	ImageName string `gorm:"column:ImageName"`
}
