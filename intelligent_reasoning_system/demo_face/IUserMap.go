package demo_face

type IUserMap interface{
	//获取用户名
	GetUserName() string
	//获取对应设备号
	GetDeviceId() string
}
