package demo_server

type Image struct {
	//所属用户
	Username string `gorm:"type:varchar(255)"`
	//图片Id
	Imageid string `gorm:"type:varchar(255)"`
	//图片名称
	Imagename string `gorm:"type:varchar(255)"`
}

//获得所属用户
func (image *Image) getUserName() string {
	return image.Username
}

//获得图片Id
func (image *Image) getImageId() string {
	return image.Imageid
}

//获得图片名称
func (image *Image) getImageName() string {
	return image.Imagename
}
