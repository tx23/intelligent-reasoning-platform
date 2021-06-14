package demo_server

type UserMap struct{
	Username string
	Deviceid string
}

//获取用户名
func (userMap *UserMap) GetUserName() string {
	return userMap.Username
}

//获取对应设备号
func (userMap *UserMap) GetDeviceId() string {
	return userMap.Deviceid
}
