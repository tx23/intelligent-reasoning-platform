package demo_face

import (
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
)

type IUser interface {
	//新增用户
	InsertUser(db *gorm.DB)
	//删除用户
	DeleteUser(db *gorm.DB)
	//查询用户
	FindUser(db *gorm.DB) IUser
	//获得UserName
	GetUserName() string
	//获得PassWord
	GetPassWord() string
	//获取设备申请管理器
	//GetUserMapManager() IUserMapManager
	//申请设备
	ApplyForDevice(db *gorm.DB) gin.HandlerFunc
	//移除设备
	RemoveDevice(db *gorm.DB) gin.HandlerFunc
	//查询设备
	ListDevice(db *gorm.DB) gin.HandlerFunc
	//查找控制组指定设备
	FindUserDevice(userMap IUserMap, db *gorm.DB) IUserMap
	//
	ApplyforRecord(db *gorm.DB) gin.HandlerFunc
}
