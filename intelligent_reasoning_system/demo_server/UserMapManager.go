package demo_server

import (
	"fmt"
	"github.com/jinzhu/gorm"
	"irs/demo_face"
)

type UserMapManager struct {}

//添加设备
func (userMapManager *UserMapManager) AddUserDevice(userMap demo_face.IUserMap, db *gorm.DB) {
	var d Device
	db.Table("device").Where("deviceid = ?", userMap.GetDeviceId()).Find(&d)
	if d.GetDeviceID() == "" {
		fmt.Println("No such device!")
	}else {
		um := UserMap{Username: userMap.GetUserName(), Deviceid: userMap.GetDeviceId()}

		userMapFind := userMapManager.FindUserDevice(&um, db)
		if userMapFind != nil{
			fmt.Println("You have applied the device before!")
		}else {
			db.Table("userdevice").Create(um)
			fmt.Println("Successfully applied the device")
		}
	}
}

//移除设备
func (userMapManager *UserMapManager) DeleteUserDevice(userMap demo_face.IUserMap, db *gorm.DB) {

	um := UserMap{Username: userMap.GetUserName(), Deviceid: userMap.GetDeviceId()}
	db.Table("userdevice").Delete(um)
	fmt.Println("Successfully remove the device!")
}

//查询用户设备
func (userMapManager *UserMapManager) FindUserDevice(userMap demo_face.IUserMap, db *gorm.DB) demo_face.IUserMap{
	//查找表
	var um UserMap
	db.Table("userdevice").Where("username = ? And deviceid = ?", userMap.GetUserName(), userMap.GetDeviceId()).Find(&um)
	fmt.Println("Find userdevice is ", um)

	if um.GetUserName() != "" {
		return &um
	}else{
		return nil
	}
}

//显示用户设备
func (userMapManager *UserMapManager) ListUserDevice(user demo_face.IUser, db *gorm.DB) {
	fmt.Println("Start listing")
	var userMap []UserMap
	db.Table("userdevice").Where("username = ?", user.GetUserName()).Find(&userMap)
	//m := make(map[string]string, len(userMap))
	fmt.Println("The userMap is: ", userMap)


	var device []Device
	var findDevice Device
	for i := 0; i < len(userMap); i++{
		db.Table("device").Where("deviceid = ?", userMap[i].GetDeviceId()).Find(&findDevice)

		fmt.Println("Find device is: ", findDevice)

		device = append(device, findDevice)
	}
	fmt.Println("Results of devices are: ", device)
}
