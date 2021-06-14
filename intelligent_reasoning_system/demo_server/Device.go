package demo_server

import (
	"irs/demo_face"
)

//Device Device设备
type Device struct {
	//设备号
	Deviceid string `json:"id"`
	//设备名
	Devicename string `json:"devicename"`
	//设备IP
	Deviceip string `json:"ip"`
}

type DataDevice struct {
	Available []Device `json:"available_list"`
	Compare   []Device `json:"compare_list"`
}

func NewDevice(deviceID string, deviceName, deviceIP string) demo_face.IDevice {
	return &Device{
		Deviceid:   deviceID,
		Devicename: deviceName,
		Deviceip:   deviceIP,
	}
}

//获得DeviceID
func (device *Device) GetDeviceID() string {
	return device.Deviceid
}

//获得DeviceName
func (device *Device) GetDeviceName() string {
	return device.Devicename
}

//获得DeviceIP
func (device *Device) GetDeviceIP() string {
	return device.Deviceip
}
