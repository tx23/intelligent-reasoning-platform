package demo_face

//Device Device设备
type IDevice interface {
	//获得DeviceID
	GetDeviceID() string
	//获得DeviceName
	GetDeviceName() string
	//获得DeviceIP
	GetDeviceIP() string
}
