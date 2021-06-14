package demo_face

import "github.com/jinzhu/gorm"

type IUserMapManager interface {
	//添加设备
	AddUserDevice(userMap IUserMap, db *gorm.DB)
	//移除设备
	DeleteUserDevice(userMap IUserMap, db *gorm.DB)
	//查询用户设备
	FindUserDevice(userMap IUserMap, db *gorm.DB) IUserMap
	//显示用户设备
	ListUserDevice(user IUser, db *gorm.DB)
}
