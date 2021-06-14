package demo_face

import (
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
)

type IDeviceManager interface {
	//添加设备
	AddDevice(db *gorm.DB) gin.HandlerFunc
	//移除设备
	RemoveDevice(db *gorm.DB) gin.HandlerFunc
	//查找设备
	FindDevice(db *gorm.DB, device IDevice) IDevice
}
