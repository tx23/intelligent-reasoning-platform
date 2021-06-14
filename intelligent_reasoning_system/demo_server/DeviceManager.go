package demo_server

import (
	"fmt"
	"irs/demo_face"

	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
)

//DeviceManager 设备管理器
type DeviceManager struct{}

func NewDeviceManager() demo_face.IDeviceManager {
	return &DeviceManager{}
}

//添加设备
func (dm *DeviceManager) AddDevice(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		fmt.Println("Start adding device!")
		//获取设备信息
		deviceID := c.PostForm("deviceID")
		deviceName := c.PostForm("deviceName")
		deviceIP := c.PostForm("deviceIP")

		d := Device{Deviceid: deviceID, Devicename: deviceName, Deviceip: deviceIP}
		fmt.Println(d)

		deviceFind := dm.FindDevice(db, &d)
		if deviceFind != nil {
			fmt.Println("Device exits!")
		} else {
			db.Table("device").Create(d)
			fmt.Println("Successfully Add Device!")
		}

	}
}

//移除设备
func (dm *DeviceManager) RemoveDevice(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		//获取设备信息
		deviceID := c.PostForm("deviceID")
		deviceName := c.PostForm("deviceName")
		deviceIP := c.PostForm("deviceIP")

		d := Device{Deviceid: deviceID, Devicename: deviceName, Deviceip: deviceIP}
		fmt.Println(d)

		deviceFind := dm.FindDevice(db, &d)
		if deviceFind != nil {
			db.Table("device").Delete(d)
			fmt.Println("Successfully delete device!")
		} else {
			fmt.Println("Device does not exits!")
		}
	}
}

//查找设备
func (dm *DeviceManager) FindDevice(db *gorm.DB, device demo_face.IDevice) demo_face.IDevice {
	var d Device
	db.Table("device").Where("deviceid = ?", device.GetDeviceID()).Find(&d)
	fmt.Println("Find device is ", d)
	if d.GetDeviceID() != "" {
		return &d
	} else {
		return nil
	}
}
