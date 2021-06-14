package demo_face

type IImage interface {
	//获得所属用户
	getUserName() string
	//获得图片Id
	getImageId() uint32
	//获得图片名称
	getImageName() string
}
