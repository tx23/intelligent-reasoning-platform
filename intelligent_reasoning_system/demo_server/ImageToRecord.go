package demo_server

type ImageToRecord struct {
	Id        string `json:"id" gorm:"column:Id"`
	ImageName        string `json:"imageName" gorm:"column:ImageName"`
}


func (imageToRecord *ImageToRecord) getId() string {
	return imageToRecord.Id
}
func (imageToRecord *ImageToRecord) getImageName() string {
	return imageToRecord.ImageName
}