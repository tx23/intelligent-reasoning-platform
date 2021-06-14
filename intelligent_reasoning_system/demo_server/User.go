package demo_server

import (
	"bytes"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"github.com/tarm/serial"
	"io/ioutil"
	"irs/demo_face"
	"irs/utils"
	"log"
	"net"
	"net/http"
	"os"
	"sort"
	"strconv"
	"time"

	sessions2 "github.com/gin-contrib/sessions"
	"github.com/gin-gonic/gin"

	_ "github.com/fogleman/gg"
	"github.com/jinzhu/gorm"
)

type User struct {
	Username string //`form:"username" json:"username" binding:"required"`
	Password string //`form:"password" json:"password" binding:"required"`
	//UserMapManager UserMapManager // 设备申请管理器
}

var ErrorFlag = false
var PowerFlag = false
var LoginFlag = false


func NewUser(Username, Password string) demo_face.IUser {
	return &User{
		Username: Username,
		Password: Password,
		//UserMapManager: UserMapManager{},
	}
}

//获取username
func (user *User) GetUserName() string {
	return user.Username
}

//获取password
func (user *User) GetPassWord() string {
	return user.Password
}

//获取设备申请管理器
//func (user *User) GetUserMapManager() demo_face.IUserMapManager {
//	return &user.UserMapManager
//}

//查找用户
func (user *User) FindUser(db *gorm.DB) demo_face.IUser {
	var u User
	db.Table("users").Where("username = ?", user.GetUserName()).Find(&u)
	fmt.Println(u)
	if u.Username != "" {
		return &u
	} else {
		return nil
	}
}

//插入用户
func (user *User) InsertUser(db *gorm.DB) {
	u := user.FindUser(db)

	if u != nil {
		fmt.Println("User exits!")
		return
	} else {
		username := user.GetUserName()
		password := user.GetPassWord()

		var u1 User
		u1 = User{Username: username, Password: password}
		db.Table("users").Create(u1)

		fmt.Printf("Insert User(%s, %s) successfully!\n", user.Username, user.Password)
	}
}

//删除用户, 没用
func (user *User) DeleteUser(db *gorm.DB) {
	u := user.FindUser(db)
	if u == nil {
		fmt.Println("User does not exits!")
	} else {
		//db.AutoMigrate(&User{})
		var u1 User
		u1 = User{Username: user.GetUserName(), Password: user.GetPassWord()}
		db.Table("users").Delete(u1)

		fmt.Printf("Delete User(%s, %s) successfully!\n", user.Username, user.Password)
	}
}

func (user *User) GetDeviceRecord(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		var userMap []UserMap
		db.Table("userdevice").Where("username = ?", user.GetUserName()).Find(&userMap)
		fmt.Println("The userMap is: ", userMap)

		var device []Device //对比设备列表
		var findDevice Device
		for i := 0; i < len(userMap); i++ {
			db.Table("device").Where("deviceid = ?", userMap[i].GetDeviceId()).Find(&findDevice)

			fmt.Println("Find deaavice is: ", findDevice)

			device = append(device, findDevice)
		}
		sort.Slice(device, func(i, j int) bool {
			s1, _ := strconv.Atoi(device[i].Deviceid)
			s2, _ := strconv.Atoi(device[j].Deviceid)
			return  s1 < s2
		})
		fmt.Println("Results of devices are: ", device)
		var allDevice []Device
		db.Table("device").Find(&allDevice)

		dataDevice := DataDevice{
			Available: allDevice,
			Compare:   device,
		}
		jsonData, _ := json.Marshal(dataDevice)

		c.Data(http.StatusOK, "application/json", jsonData)
		fmt.Println("Get Device!")
		c.Abort()
	}
}

//申请设备
func (user *User) ApplyForDevice(db *gorm.DB) gin.HandlerFunc {
	fmt.Println("Start applying for device!")
	return func(c *gin.Context) {
		sessions := sessions2.Default(c)

		if sessions.Get(user.GetUserName()) != user.GetUserName() {
			fmt.Println("You have login out")
			LoginFlag = true
			c.Next()
		} else {
			//获取device id
			type DeviceID struct {
				Data string `json:"data"`
			}
			var tempId DeviceID
			if err := c.ShouldBindJSON(&tempId); err != nil {
				c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
				return
			}
			deviceID := tempId.Data

			fmt.Println("username is: ", user.GetUserName(), " deviceID is: ", deviceID) // Test

			userMap := &UserMap{Username: user.GetUserName(), Deviceid: deviceID}
			fmt.Println("userMap: ", userMap)
			fmt.Println(userMap.GetDeviceId())
			//user.GetUserMapManager().AddUserDevice(&userMap, db)
			var d Device
			db.Table("device").Where("deviceId = ?", userMap.GetDeviceId()).Find(&d)
			if d.GetDeviceID() == "" {
				fmt.Println("No such device!")
			} else {
				um := UserMap{Username: userMap.GetUserName(), Deviceid: userMap.GetDeviceId()}

				userMapFind := user.FindUserDevice(&um, db)
				if userMapFind != nil {
					fmt.Println("You have applied the device before!")
				} else {
					db.Table("userdevice").Create(um)
					fmt.Println("Successfully applied the device!")
					c.Next()
				}
			}
		}
	}
}

//移除设备
func (user *User) RemoveDevice(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		sessions := sessions2.Default(c)

		if sessions.Get(user.GetUserName()) != user.GetUserName() {
			fmt.Println("You have login out")
		} else {
			//获取device id
			type DeviceID struct {
				Data string `json:"data"`
			}
			var tempId DeviceID
			if err := c.ShouldBindJSON(&tempId); err != nil {
				c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
				return
			}
			deviceID := tempId.Data

			um := UserMap{Username: user.GetUserName(), Deviceid: deviceID}
			db.Table("userdevice").Where("username = ? AND deviceid = ?", um.GetUserName(), um.GetDeviceId()).Delete(&um)
			fmt.Println("Successfully remove the device!")
			c.Next()
		}
	}
}

//显示设备
func (user *User) ListDevice(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		sessions := sessions2.Default(c)

		if sessions.Get(user.GetUserName()) != user.GetUserName() {
			fmt.Println("You have login out")
		} else {
			fmt.Println("Before list!")
			//user.GetUserMapManager().ListUserDevice(user, db)
			fmt.Println("Start listing!")
			c.Next()
		}
	}
}

//查找控制组指定设备
func (user *User) FindUserDevice(userMap demo_face.IUserMap, db *gorm.DB) demo_face.IUserMap {
	//查找表
	var um UserMap
	db.Table("userdevice").Where("username = ? AND deviceid = ?", userMap.GetUserName(), userMap.GetDeviceId()).Find(&um)
	fmt.Println("Find userdevice is ", um)

	if um.GetUserName() != "" {
		return &um
	} else {
		return nil
	}
}

//性能测试初始数据(返回数据库数据)
// TODO: To Change
func (user *User) PerformanceOrigin(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		var record []Record
		var result []Record
		db.Table("record").Last(&record)
		batchNum := record[0].Batch
		db.Table("record").Where("batch = ?", batchNum).Find(&result)
		jsonData, _ := json.Marshal(result)
		fmt.Println("---------------------Origin Data: ", string(jsonData))
		c.Data(http.StatusOK, "application/json", jsonData)
		//c.Abort()
	}
}

//初始化数据返回HistoryOrigin
func (user *User) HistoryOrigin(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		var record []Record
		db.Table("record").Find(&record)
		for i := 0; i < len(record)/2; i++ {
			record[i], record[len(record)-i-1] = record[len(record)-i-1], record[i]
		}

		jsonData, _ := json.Marshal(record)
		//fmt.Println(string(jsonData))
		c.Data(http.StatusOK, "application/json", jsonData)
	}
}

// Search last Batch Num in database
func searchLastBatchNum(db *gorm.DB) int {
	var lastRecord []Record
	db.Table("record").Last(&lastRecord)
	if len(lastRecord) == 0 {
		return 0
	}

	result, _ := strconv.Atoi(lastRecord[0].Batch)
	return result + 1

}

func getKindOfModel(c *gin.Context) string{
	type KindOfAl struct {
		KindOfModel string `json:"kindOfModel"`
	}
	var al KindOfAl
	if err := c.ShouldBindJSON(&al); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return "err"
	}

	if al.KindOfModel == "请选择模型" {
		return "1" // 默认采用算法1
	}
	return al.KindOfModel
}

// Read Power
func readPower(config *serial.Config, ch chan bool, power *[4]float64, size int) {
	defer func() {
		if err := recover(); err != nil {
			PowerFlag = true
			<-ch
			fmt.Println("In Read 1H8 goroutine recover: ", err)
			return
		}
	}()

	port, err := serial.OpenPort(config)
	if err != nil {
		fmt.Println("1H8设备功耗读取失败，失败原因: 无法打开背板串口")
	}
	defer port.Close()

	dataBuff := bytes.NewBuffer([]byte{})
	var slaveId uint8
	slaveId = 0x01
	if err := binary.Write(dataBuff, binary.LittleEndian, slaveId); err != nil {
		fmt.Println("Write error!")
	}

	data := dataBuff.Bytes()
	fmt.Println(data)
	for {
		select {
		case <-ch:
			return
		default:
			buf := make([]byte, size)
			count, err1 := port.Read(buf)

			if err1 != nil {
				log.Println("err1: ", err1)
				break
				//fmt.Println("1H8设备功耗读取失败，失败原因：成功打开串口，但无法读取数据")
			} else if count == size {

				for i := 0; i < 4; i++ {
					start := size / 5 - i - 1
					cardPower, err := strconv.ParseFloat(string(buf[start*5:(start+1)*5]), 64)
					if err != nil {
						fmt.Println("Read power failed!")
					}
					if cardPower >  power[4-i-1]{
						if cardPower > 10 {
							temp := cardPower - float64(int(cardPower))
							cardPower = 10 - temp
						}
						power[4-i-1] = cardPower
					}
				}
			}
		}
	}
}

func cutSlice(s string) []string{
	start := 0
	end := start
	var result []string
	for i := 0; i < len(s); i++ {
		if s[i] == '[' {
			start = i+1
		} else if s[i] == ']' {
			end = i
			result = append(result, s[start:end])
		}
	}
	return result
}

func getDetections(s []string, detection *[]Detection) {
	for i := 0; i < len(s); i++ {

		var stringTemp []string
		start := 0
		for j := 0; j < len(s[i]); j++ {
			if s[i][j] == ' ' {
				stringTemp = append(stringTemp, s[i][start:j])
				start = j+1
			}
		}
		stringTemp = append(stringTemp, s[i][start:])
		fmt.Println("...............stringTemp: ", stringTemp)
		confidence, _ := strconv.ParseFloat(stringTemp[1], 10)
		x1, _ := strconv.ParseFloat(stringTemp[2], 10)
		y1, _ := strconv.ParseFloat(stringTemp[3], 10)
		x2, _ := strconv.ParseFloat(stringTemp[4], 10)
		y2, _ := strconv.ParseFloat(stringTemp[5], 10)

		temp := Detection{
			Label: stringTemp[0],
			Confidence: confidence,
			X1: x1,
			Y1: y1,
			X2: x2,
			Y2: y2,
		}
		*detection = append(*detection, temp)
	}
}



// 提交测试请求
func (user *User) ApplyForTest(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		utils.ImageObj.Reload()

		batchNum := searchLastBatchNum(db)

		defer func() {
			if err := recover(); err != nil {
				ErrorFlag = true
				return
			}
		}()

		sessions := sessions2.Default(c)

		kindOfModel := getKindOfModel(c)
		if kindOfModel == "err" {
			return
		}

		SelectMap := make(map[string]string)
		var dir string
		if  kindOfModel == "1" {
			SelectMap = utils.ImageMap_Yaogan
			dir = utils.PathObj.TestPath2
		}else {
			SelectMap = utils.ImageMap1
			dir = utils.PathObj.TestPath
		}


		fmt.Println("获取的算法值: ", kindOfModel)

		if sessions.Get(user.GetUserName()) != user.GetUserName() {
			fmt.Println("You have login out")
			c.HTML(http.StatusOK, "login.html", nil)
		} else {
			// 查询用户拥有的设备情况
			var userMap []UserMap
			db.Table("userdevice").Where("username = ?", user.GetUserName()).Find(&userMap)
			fmt.Println("The userMap is: ", userMap)
			if len(userMap) == 0 {
				fmt.Println("Please choose the device")
				return
			}

			var device []Device
			var findDevice Device
			for i := 0; i < len(userMap); i++ {
				db.Table("device").Where("deviceid = ?", userMap[i].GetDeviceId()).Find(&findDevice)

				fmt.Println("Find device is: ", findDevice)

				device = append(device, findDevice)
			}
			fmt.Println("Results of devices are: ", device)

			netId := c.PostForm("netId")

			files, err := ioutil.ReadDir(dir)
			if err != nil {
				fmt.Println("Read test files failed")
			}

			//并发控制
			countDevice := len(device)
			var ch = make(chan int)
			wholeRecord := make([]Record, len(device))

			// Nvidia IoT device port config
			config1 := serial.Config{
				Name:        "/dev/ttyUSB2",
				Baud:        115200,
				Size:        8,
				StopBits:    1,
				Parity:      serial.ParityNone,
				ReadTimeout: 500 * time.Millisecond,
			}
			//powerN1 := PowerN{
			//	Card1: 0,
			//	Card2: 0,
			//	Card3: 0,
			//	Card4: 0,
			//}
			// 1H8 device port config
			config2 := serial.Config{
				Name:        "/dev/ttyUSB0",
				Baud:        115200,
				Size:        8,
				StopBits:    1,
				Parity:      serial.ParityNone,
				ReadTimeout: 500 * time.Millisecond,
			}
			//powerN2 := PowerN{
			//	Card1: 0,
			//	Card2: 0,
			//	Card3: 0,
			//	Card4: 0,
			//}
			ch0 := make(chan bool)
			ch1 := make(chan bool)
			ch2 := make(chan bool)
			ch3 := make(chan bool)

			var powerN1 [4]float64
			var powerN2 [4]float64

			/*Read 1H8 device power*/
			go readPower(&config2, ch0, &powerN1, 40)
			/*Read Nvidia device power*/
			go readPower(&config1, ch1, &powerN2, 35)

			var mluPower1 int32
			var mluPower2 int32

			readMluPowerPort := "9089"

			go func(mluPower *int32) {
				fmt.Println("Enter Read MlU1 Power goroutine")
				defer func() {
					if err := recover(); err != nil {
						PowerFlag = true
						fmt.Println("In MlU Read goroutine: ", err)
						<-ch2
						return
						//c.HTML(http.StatusOK, "performance_monitor.html", nil)
					}
				}()
				//连接
				ip := utils.MapHost["mlu100-1"].Deviceip
				address := ip + ":" + readMluPowerPort
				for {
					select {
					case <-ch2:
						return
					default:
						conn, err := net.Dial("tcp", address)
						if err != nil {
							fmt.Println("读取功耗时，mlu100-1设备连接失败")
							fmt.Println("Connection failed: ", address)
						}
						defer conn.Close()
						readPower := make([]byte, 4)
						if _, err = conn.Read(readPower); err != nil {
							fmt.Println("Read FileId failed!")
						}
						readPowerBuf := bytes.NewBuffer(readPower)
						var to32_readPower int32
						binary.Read(readPowerBuf, binary.LittleEndian, &to32_readPower)
						if *mluPower < to32_readPower {
							*mluPower = to32_readPower
						}
						time.Sleep(500 * time.Millisecond)
					}
				}
			}(&mluPower1)

			go func(mluPower *int32) {
				fmt.Println("Enter Read MlU2 Power goroutine")

				defer func() {
					if err := recover(); err != nil {
						PowerFlag = true
						fmt.Println("In MlU Read goroutine: ", err)
						<-ch3
						return
					}
				}()
				//连接
				ip := utils.MapHost["mlu100-2"].Deviceip
				address := ip + ":" + readMluPowerPort
				for {
					select {
					case <-ch3:
						return
					default:
						conn, err := net.Dial("tcp", address)
						if err != nil {
							fmt.Println("读取功耗时，mlu100-2设备连接失败")
							// TODO: 前端显示相关信息，返回测试请求相关页面
						}
						defer conn.Close()
						readPower := make([]byte, 4)
						if _, err = conn.Read(readPower); err != nil {
							fmt.Println("Read FileId failed!")
						}
						readPowerBuf := bytes.NewBuffer(readPower)
						var to32_readPower int32
						binary.Read(readPowerBuf, binary.LittleEndian, &to32_readPower)
						//fmt.Println("reFileId: ", to32_readPower)
						if *mluPower < to32_readPower {
							*mluPower = to32_readPower
						}
						fmt.Println("mluPower: ", *mluPower)
						time.Sleep(500 * time.Millisecond)
					}
				}
			}(&mluPower2)

			for i := 0; i < len(device); i++ {
				//log.Println("now Ip is: ", device[i].GetDeviceIP())
				log.Println("loop? ", i)
				//log.Fatalln("haha")
				if ((device[i].GetDeviceIP() == utils.MapHost["1h8_2-1"].Deviceip) || (device[i].GetDeviceIP() == utils.MapHost["1h8_2-2"].Deviceip)) &&
					((kindOfModel == "3") || (kindOfModel == "4")) {
					go func(is int) {
						wholeRecord[is] = Record{
							Id: "1H8(2核心)不支持" + utils.NumToModel[kindOfModel] + "推理",
						}
						ch <- is
					}(i)

					continue
				}
				if (device[i].GetDeviceIP() == utils.MapHost["mlu100-1"].Deviceip && kindOfModel == "1") ||
					(device[i].GetDeviceIP() == utils.MapHost["mlu100-2"].Deviceip && kindOfModel == "1") {
					go func(is int) {
						defer func() {
							if err := recover(); err != nil {
								ErrorFlag = true
								ch <- is
								fmt.Println("In MlU Read Normal Data goroutine: ", err)
								return
							}
						}()
						timeStart := time.Now()
						var count int
						var wholeInferTime float64
						IdMessage := time.Now().Format("2006-01-02_15:04:05") + "_" + device[is].Devicename

						for _, f := range files {
							count++

							//Open Images
							source := make([]byte, 500000)
							ff, err := os.Open(dir + f.Name())
							if err != nil {
								fmt.Println("读取图片失败")
							}

							// Read Images into streams
							n, errF := ff.Read(source)
							if errF != nil {
								fmt.Println("Read image failed!")
							}
							sourceBuffer := source[:n]

							//base64 encoding
							base64Image := base64.StdEncoding.EncodeToString(sourceBuffer)

							//json Marshal
							jsonImage := MluResult{
								Image: base64Image,
							}

							jsonRequest, errJson := json.Marshal(jsonImage)
							if errJson != nil {
								fmt.Println("Json Marshal failed!")
							}
							jsonPost := []byte(jsonRequest)

							//http request
							url := "http://" + device[is].GetDeviceIP() + ":" + utils.ModelMap["Yolov3_satellite"] + "/add_json"
							req, errHttp := http.NewRequest("POST", url, bytes.NewBuffer(jsonPost))
							if errHttp != nil {
								fmt.Println("mlu100-1的http请求失败")
								// TODO: 前端显示信息
							}
							req.Header.Set("Content-Type", "application/json")
							client := &http.Client{}
							resp, errResp := client.Do(req)
							if errResp != nil {
								fmt.Println("mlu100-1的http请求失败")
								// TODO: 前端显示信息
							}
							defer resp.Body.Close()

							readBody, _ := ioutil.ReadAll(resp.Body)

							body := MluResult{}
							errjsonBody := json.Unmarshal(readBody, &body)
							if errjsonBody != nil {
								log.Fatalln(errjsonBody)
							}


							transTime, errTrans := strconv.ParseFloat(body.Time, 64)
							if errTrans != nil {
								fmt.Println("Convert failed!")
							}
							wholeInferTime += transTime

							//finalImage := body.Image
							boxes := body.Boxes

							cutboxes := cutSlice(boxes)
							var finalBoxes []Detection
							getDetections(cutboxes, &finalBoxes)

							var draw Draw

							point := make([]Point, len(finalBoxes))
							for j := 0; j < len(finalBoxes); j++ {
								point[j].x1 = float32(finalBoxes[j].X1)
								point[j].y1 = float32(finalBoxes[j].Y1)
								point[j].x2 = float32(finalBoxes[j].X2)
								point[j].y2 = float32(finalBoxes[j].Y2)
							}


							fmt.Println("Start drawing frame on the image... ...")
							imgName := draw.drawRec(dir+f.Name(), device[is].GetDeviceID(), f.Name(), point)
							var recordData RecordData
							recordData = RecordData{
								Time:       time.Now().Format("2006-01-02"),
								DeviceName: device[is].GetDeviceName(),
								ImgName:    imgName,
							}
							db.Table("imgRecord").Create(recordData)
							var recordToImage RecordToImage
							recordToImage = RecordToImage{
								Id:        IdMessage,
								ImageName: imgName,
							}
							db.Table("imageToRecord").Create(recordToImage)

							fmt.Println(".....................Origin boxes: ", boxes)
							fmt.Println(".....................MLU infer boxes: ", cutboxes)
							//finalTime := body.Time
							//final, _ := base64.StdEncoding.DecodeString(finalImage)

							//currentTime := time.Now().Format("2006-01-02_15:04:05")
							//imgName := device[is].GetDeviceID() + "_" + f.Name()[:len(f.Name())-4] + "_" + currentTime + ".png"

							//file, errFile := os.OpenFile(utils.PathObj.RecordPath+imgName,
							//	os.O_CREATE|os.O_WRONLY, 0666)
							fmt.Println(body.Time)
							//if errFile != nil {
							//	fmt.Println("Open File failed!")
							//}
							//
							//defer file.Close()
							//_, errWrite := file.Write(final)
							//if errWrite != nil {
							//	fmt.Println("Write File failed!")
							//	log.Fatalln(errWrite)
							//}

							//var recordData RecordData
							//recordData = RecordData{
							//	Time:       time.Now().Format("2006-01-02"),
							//	DeviceName: device[is].GetDeviceName(),
							//	ImgName:    imgName,
							//}
							//db.Table("imgRecord").Create(recordData)
							//
							//recordToImage := RecordToImage{
							//	Id:        IdMessage,
							//	ImageName: imgName,
							//}
							//db.Table("imageToRecord").Create(recordToImage)
						}
						wholeTransTime := time.Since(timeStart)
						AverageInferTime := wholeInferTime / float64(count)
						fps := 1.0 / AverageInferTime
						var power float64
						if device[is].GetDeviceIP() == utils.MapHost["mlu100-1"].Deviceip {
							power = float64(mluPower1)
						}
						if device[is].GetDeviceIP() == utils.MapHost["mlu100-2"].Deviceip {
							power = float64(mluPower2)
						}
						var record Record
						record = Record{
							//Ip:         device[is].GetDeviceIP(),
							Ip:	        utils.NumToModel[kindOfModel],
							Type:       device[is].GetDeviceName(),
							Ptime:      strconv.FormatFloat(wholeInferTime, 'f', 2, 64),
							Ctime:      strconv.FormatFloat(wholeTransTime.Seconds(), 'f', 2, 64),
							Fps:        strconv.FormatFloat(fps, 'f', 2, 64),
							PowerWaste: strconv.FormatFloat(power, 'f', 2, 64),
							MemUsage:   "",
							Id:         IdMessage,
							Batch:      strconv.Itoa(batchNum),
						}
						db.Table("record").Create(record)

						//wholeRecord = append(wholeRecord, record)
						wholeRecord[is] = record

						ch <- is
					}(i)
				} else if device[i].GetDeviceIP() == utils.MapHost["1h8_2-1"].Deviceip ||
					device[i].GetDeviceIP() == utils.MapHost["1h8_2-2"].Deviceip ||
					device[i].GetDeviceIP() == utils.MapHost["1h8_4-1"].Deviceip ||
					device[i].GetDeviceIP() == utils.MapHost["1h8_4-2"].Deviceip {

					var port int64
					port = 7000

					if device[i].GetDeviceIP() == utils.MapHost["1h8_4-2"].Deviceip {
						tmp, _ := strconv.ParseInt(kindOfModel, 10, 64)
						port += tmp
					}
					if device[i].GetDeviceIP() == utils.MapHost["1h8_4-1"].Deviceip {
						tmp, _ := strconv.ParseInt(kindOfModel, 10, 64)
						port = port + 4 + tmp
					}
					if device[i].GetDeviceIP() == utils.MapHost["1h8_2-2"].Deviceip {
						tmp, _ := strconv.ParseInt(kindOfModel, 10, 64)
						port = port + 8 + tmp
					}
					if device[i].GetDeviceIP() == utils.MapHost["1h8_2-1"].Deviceip {
						tmp, _ := strconv.ParseInt(kindOfModel, 10, 64)
						port = port + 12 + tmp
					}

					// Connection build
					ip := "192.168.3.10"
					address := ip + ":" + strconv.FormatInt(port, 10)


					go func(is int) {
						defer func() {
							if err := recover(); err != nil {
								ErrorFlag = true
								fmt.Println("In Computing Time goroutine: ", err)
								ch <- is
								return
							}
						}()

						IdMessage := time.Now().Format("2006-01-02_15:04:05") + "_" + device[is].Devicename

						startTime := time.Now()
						conn, err := net.Dial("tcp", address)
						if err != nil {
							fmt.Println("1H8控制台连接失败")
						}
						defer conn.Close()

						// Read Data from 1H8
						wholeInferTime := 0.00
						for j := 0; j < len(SelectMap); j++ {

							//读取FileId
							reFileId := make([]byte, 8)
							if _, err = conn.Read(reFileId); err != nil {
								fmt.Println("Read FileId failed!")
							}
							reFileIdBuf := bytes.NewBuffer(reFileId)
							var to64_reFileId uint64
							binary.Read(reFileIdBuf, binary.LittleEndian, &to64_reFileId)

							// Read Count of frames
							//读取框的个数
							reCountFrame := make([]byte, 8)
							if _, err = conn.Read(reCountFrame); err != nil {
								fmt.Println("Read count of frame failed!")
							}
							reCountFrameBuf := bytes.NewBuffer(reCountFrame)
							var to64_reCountFrame int64
							binary.Read(reCountFrameBuf, binary.LittleEndian, &to64_reCountFrame)
							fmt.Println("reCountFrame: ", to64_reCountFrame)

							//读取框框数据
							if to64_reCountFrame != 0 {
								reFrameData := make([]byte, 16*to64_reCountFrame)
								if _, err = conn.Read(reFrameData); err != nil {
									fmt.Println("Read frame data failed!")
								}
								var j int64
								point := make([]Point, to64_reCountFrame)
								for j = 0; j < to64_reCountFrame; j++ {
									frameData := reFrameData[j*16 : (j+1)*16]
									// 获取坐标
									var x1, y1, x2, y2 float32
									x1_byte := frameData[:4]
									y1_byte := frameData[4:8]
									x2_byte := frameData[8:12]
									y2_byte := frameData[12:16]
									x1_Buf := bytes.NewBuffer(x1_byte)
									y1_Buf := bytes.NewBuffer(y1_byte)
									x2_Buf := bytes.NewBuffer(x2_byte)
									y2_Buf := bytes.NewBuffer(y2_byte)
									binary.Read(x1_Buf, binary.LittleEndian, &x1)
									binary.Read(y1_Buf, binary.LittleEndian, &y1)
									binary.Read(x2_Buf, binary.LittleEndian, &x2)
									binary.Read(y2_Buf, binary.LittleEndian, &y2)
									temp_point := Point{x1: x1, y1: y1, x2: x2, y2: y2}
									point[j] = temp_point
									//fmt.Printf("Frame is (%f, %f), (%f, %f)\n", x1, y1, x2, y2)
								}
								//画框
								fmt.Println("Len of Point is: ", len(point))
								draw := Draw{}
								var image Image
								str_reFileId := strconv.FormatUint(to64_reFileId, 10)
								image = Image{
									Username: user.GetUserName(),
									Imageid: str_reFileId,
									Imagename: SelectMap[str_reFileId],
								}
								if image.getImageName() != "" {
									fmt.Println("Start drawing frame on the image... ...")
									imgName := draw.drawRec(dir+image.getImageName(), device[is].Deviceid, image.getImageName(), point)
									var recordData RecordData
									recordData = RecordData{
										Time:       time.Now().Format("2006-01-02"),
										DeviceName: device[is].GetDeviceName(),
										ImgName:    imgName,
									}
									db.Table("imgRecord").Create(recordData)
									var recordToImage RecordToImage
									recordToImage = RecordToImage{
										Id:        IdMessage,
										ImageName: imgName,
									}
									db.Table("imageToRecord").Create(recordToImage)
								} else {
									fmt.Println("User doesn't have the image!")
								}
							}

							// Read Infer Time
							reTime := make([]byte, 8)
							if _, err = conn.Read(reTime); err != nil {
								fmt.Println("Read time failed!")
							}
							reTimeBuf := bytes.NewBuffer(reTime)
							var to64_reTime float64
							binary.Read(reTimeBuf, binary.LittleEndian, &to64_reTime)
							fmt.Println("reTime: ", to64_reTime)
							wholeInferTime += to64_reTime
						}

						wholeTransTime := time.Since(startTime)
						//var returnWholeInferTime float64
						AverageInferTime := wholeInferTime / float64(len(SelectMap))


						fmt.Printf("Gouroutine %d cost [%s]!\n", is, wholeTransTime)

						var power float64
						if device[is].GetDeviceIP() == utils.MapHost["1h8_2-1"].Deviceip {
							power = powerN1[0]
							//if len(SelectMap) < 2 {
							//	returnWholeInferTime = wholeInferTime / float64(len(SelectMap))
							//}else {
							//	returnWholeInferTime = wholeInferTime / 2
							//}
						}
						if device[is].GetDeviceIP() == utils.MapHost["1h8_2-2"].Deviceip {
							power = powerN1[1]
							//if len(SelectMap) < 2 {
							//	returnWholeInferTime = wholeInferTime / float64(len(SelectMap))
							//}else {
							//	returnWholeInferTime = wholeInferTime / 2
							//}
						}
						if device[is].GetDeviceIP() == utils.MapHost["1h8_4-1"].Deviceip {
							power = powerN1[2]
							//if len(SelectMap) < 4 {
							//	returnWholeInferTime = wholeInferTime / float64(len(SelectMap))
							//}else {
							//	returnWholeInferTime = wholeInferTime / 4
							//}
						}
						if device[is].GetDeviceIP() == utils.MapHost["1h8_4-2"].Deviceip {
							power = powerN1[3]
							//if len(SelectMap) < 4 {
							//	returnWholeInferTime = wholeInferTime / float64(len(SelectMap))
							//}else {
							//	returnWholeInferTime = wholeInferTime / 4
							//}
						}
						//fps := 1.0 / (returnWholeInferTime / float64(len(SelectMap)))
						fps := 1.0 / AverageInferTime

						var record Record
						record = Record{
							Ip:         utils.NumToModel[kindOfModel],
							Type:       device[is].GetDeviceName(),
							Ptime:      strconv.FormatFloat(wholeInferTime, 'f', 2, 64),
							Ctime:      strconv.FormatFloat(wholeTransTime.Seconds(), 'f', 2, 64),
							Fps:        strconv.FormatFloat(fps, 'f', 2, 64),
							PowerWaste: strconv.FormatFloat(power, 'f', 2, 64),
							MemUsage:   "",
							Id:         IdMessage,
							Batch:      strconv.Itoa(batchNum),
						}

						db.Table("record").Create(record)
						wholeRecord[is] = record

						ch <- is

					}(i)

				} else {
					go func(is int) {
						defer func() {
							if err := recover(); err != nil {
								ErrorFlag = true
								fmt.Println("In Read Normal Data2: ", err)
								ch <- is
								return
							}
						}()
						var count int
						var port string
						if kindOfModel == "1" {
							port = ":" + utils.ModelMap["Yolov3_satellite"]
						} else if kindOfModel == "2" {
							port = ":" + utils.ModelMap["Vgg16_ssd"]
						} else if kindOfModel == "3" {
							port = ":" + utils.ModelMap["MobileNetv2_ssd"]
						} else {
							port = ":" + utils.ModelMap["Yolov3"]
						}
						dataBuff := bytes.NewBuffer([]byte{})
						send_len := 0

						for _, f := range files {
							count++

							var str_count = strconv.FormatInt(int64(count), 10)
							if SelectMap[str_count] != f.Name() {
								for k, v := range SelectMap {
									if v == f.Name() {
										str_count = k
										break
									}
								}
							}
							image := Image{Username: user.GetUserName(), Imageid: str_count, Imagename: f.Name()}
							//db.Table("image").Create(image)

							//打开读取图片
							sourceBuffer := make([]byte, 500000)
							ff, err := os.Open(dir + f.Name())
							if err != nil {
								fmt.Println("打开图片失败")
								// TODO: 前端显示相关信息
							}
							n, err := ff.Read(sourceBuffer)
							if err != nil {
								fmt.Println("Read file failed!")
							}
							finalBuffer := sourceBuffer[:n]

							//数据封装+发送
							netInt, _ := strconv.ParseUint(netId, 10, 64)
							fileId, _ := strconv.ParseUint(image.getImageId(), 10, 64)
							transFile := TransFile{FileId: fileId, NetId: netInt, DataLen: uint64(len(finalBuffer)), Data: finalBuffer}

							deviceId, _ := strconv.ParseUint(device[is].GetDeviceID(), 10, 64)

							var wholeLen = 32 + len(transFile.getData())
							send_len += wholeLen
							fmt.Println("send_len is: ", send_len)

							//写deviceId
							if err := binary.Write(dataBuff, binary.LittleEndian, deviceId); err != nil {
								fmt.Println("Writing device failed!")
							}
							// 写FileId
							if err := binary.Write(dataBuff, binary.LittleEndian, transFile.getFileId()); err != nil {
								fmt.Println("Writing FileId failed!")
							}
							// 写NetId
							if err := binary.Write(dataBuff, binary.LittleEndian, transFile.getNetId()); err != nil {
								fmt.Println("Writing NetId failed!")
							}
							// 写DataLen
							if err := binary.Write(dataBuff, binary.LittleEndian, transFile.getDataLen()); err != nil {
								fmt.Println("Writing DataLen failed!")
							}
							// 写Data
							if err := binary.Write(dataBuff, binary.LittleEndian, transFile.getData()); err != nil {
								fmt.Println("Writing Data failed!")
							}
						}
						fmt.Println("Count of files are: ", count)

						//计算总长度
						send_len += 8
						var to64_send_len = int64(send_len)
						fmt.Println("Final send len is:", to64_send_len)

						//总长度写入头字节
						fmt.Println("Len of dataBuff: ", len(dataBuff.Bytes()))
						lenBuff := bytes.NewBuffer([]byte{})
						if err := binary.Write(lenBuff, binary.LittleEndian, to64_send_len); err != nil {
							fmt.Println("Writing send_len failed!")
						}

						//头字节、数据字节合并
						var buffer bytes.Buffer
						buffer.Write(lenBuff.Bytes())
						buffer.Write(dataBuff.Bytes())
						send := buffer.Bytes()
						//send := append(lenBuff.Bytes(), dataBuff.Bytes()...)
						fmt.Println("Len of send is: ", len(send))

						//连接设备
						ip := device[is].GetDeviceIP()
						deviceId := device[is].GetDeviceID()
						// ip := "192.168.3.11"
						fmt.Println("Device IP is: ", ip)
						address := ip + port
						//计时开始
						startTime := time.Now()
						conn, err := net.Dial("tcp", address)
						if err != nil {
							fmt.Println("请求连接设备失败")
							fmt.Println("Connection failed: ", address)
						}
						defer conn.Close()

						//数据写入
						if _, err = conn.Write(send); err != nil {
							fmt.Println("Write data failed!")
						}

						// 数据接收
						var wholeInferTime float64
						IdMessage := time.Now().Format("2006-01-02_15:04:05") + "_" + device[is].Devicename
						//IdMessage := time.Now().Format("2006_01_02_15_04_05") + "-" + device[is].Devicename
						for j := 0; j < count; j++ {
							//读取FileId
							reFileId := make([]byte, 8)
							if _, err = conn.Read(reFileId); err != nil {
								fmt.Println("Read FileId failed!")
							}
							//ch1 <- true
							//fmt.Println("Recv byte of fileId: ", reFileId)
							reFileIdBuf := bytes.NewBuffer(reFileId)
							var to64_reFileId uint64
							binary.Read(reFileIdBuf, binary.LittleEndian, &to64_reFileId)
							//fmt.Println("reFileId: ", to64_reFileId)

							//读取框的个数
							reCountFrame := make([]byte, 2)
							if _, err = conn.Read(reCountFrame); err != nil {
								fmt.Println("Read count of frame failed!")
							}
							reCountFrameBuf := bytes.NewBuffer(reCountFrame)
							var to16_reCountFrame int16
							binary.Read(reCountFrameBuf, binary.LittleEndian, &to16_reCountFrame)
							fmt.Println("reCountFrame: ", to16_reCountFrame)

							//读取框框数据
							if to16_reCountFrame != 0 {
								reFrameData := make([]byte, 24*to16_reCountFrame)
								if _, err = conn.Read(reFrameData); err != nil {
									fmt.Println("Read frame data failed!")
								}
								var j int16
								point := make([]Point, to16_reCountFrame)
								for j = 0; j < to16_reCountFrame; j++ {
									frameData := reFrameData[j*24 : (j+1)*24]
									// 获取坐标
									var x1, y1, x2, y2 float32
									x1_byte := frameData[:4]
									y1_byte := frameData[4:8]
									x2_byte := frameData[8:12]
									y2_byte := frameData[12:16]
									x1_Buf := bytes.NewBuffer(x1_byte)
									y1_Buf := bytes.NewBuffer(y1_byte)
									x2_Buf := bytes.NewBuffer(x2_byte)
									y2_Buf := bytes.NewBuffer(y2_byte)
									binary.Read(x1_Buf, binary.LittleEndian, &x1)
									binary.Read(y1_Buf, binary.LittleEndian, &y1)
									binary.Read(x2_Buf, binary.LittleEndian, &x2)
									binary.Read(y2_Buf, binary.LittleEndian, &y2)
									temp_point := Point{x1: x1, y1: y1, x2: x2, y2: y2}
									point[j] = temp_point
									//fmt.Printf("Frame is (%f, %f), (%f, %f)\n", x1, y1, x2, y2)
									// 获取置信度
									var confidence float32
									confidence_byte := frameData[16:20]
									confidence_Buf := bytes.NewBuffer(confidence_byte)
									binary.Read(confidence_Buf, binary.LittleEndian, &confidence)
									fmt.Println("Confidence is: ", confidence)
									// 获取类别
									var category float32
									category_byte := frameData[20:24]
									category_Buf := bytes.NewBuffer(category_byte)
									binary.Read(category_Buf, binary.LittleEndian, &category)
									fmt.Println("Category is: ", category)
								}
								//画框
								fmt.Println("Len of Point is: ", len(point))
								draw := Draw{}
								var image Image
								str_reFileId := strconv.FormatUint(to64_reFileId, 10)
								image = Image{
									Username: user.GetUserName(),
									Imageid: string(str_reFileId),
									Imagename: SelectMap[str_reFileId],
								}
								//db.Table("image").Where("username = ? AND imageid = ? ", user.GetUserName(), string(str_reFileId)).Find(&image)
								fmt.Println("Find image is: ", image)
								if image.getImageName() != "" {
									fmt.Println("Start drawing frame on the image... ...")
									imgName := draw.drawRec(dir+image.getImageName(), deviceId, image.getImageName(), point)
									var recordData RecordData
									recordData = RecordData{
										Time:       time.Now().Format("2006-01-02"),
										DeviceName: device[is].GetDeviceName(),
										ImgName:    imgName,
									}
									fmt.Println("record insert:", recordData)
									db.Table("imgRecord").Create(recordData)
									var recordToImage RecordToImage
									recordToImage = RecordToImage{
										Id:        IdMessage,
										ImageName: imgName,
									}
									db.Table("imageToRecord").Create(recordToImage)
								} else {
									fmt.Println("User doesn't have the image!")
								}
							}

							//读取每张图片推理时间
							reTime := make([]byte, 8)
							if _, err = conn.Read(reTime); err != nil {
								fmt.Println("Read time failed!")
							}
							reTimeBuf := bytes.NewBuffer(reTime)
							var to64_reTime float64
							binary.Read(reTimeBuf, binary.LittleEndian, &to64_reTime)
							fmt.Println("reTime: ", to64_reTime)
							wholeInferTime += to64_reTime

						}
						//chTotal <- true

						// Receive power
						var power float64
						if device[is].GetDeviceIP() == utils.MapHost["rk3399-201-1"].Deviceip {
							power = powerN2[0]
						}
						if device[is].GetDeviceIP() == utils.MapHost["rk3399-201-2"].Deviceip {
							power = powerN2[1]
						}
						if device[is].GetDeviceIP() == utils.MapHost["agx-201-1"].Deviceip {
							power = powerN2[2]
						}
						if device[is].GetDeviceIP() == utils.MapHost["agx-201-2"].Deviceip {
							power = powerN2[3]
						}
						if device[is].GetDeviceIP() == utils.MapHost["mlu100-1"].Deviceip {
							power = float64(mluPower1)
						}
						if device[is].GetDeviceIP() == utils.MapHost["mlu100-2"].Deviceip {
							power = float64(mluPower2)
						}

						wholeTransTime := time.Since(startTime)
						AveInferTime := wholeInferTime / float64(count)
						fps := 1.0 / AveInferTime


						fmt.Println("WholeInferTime: ", wholeInferTime)
						fmt.Printf("Gouroutine %d cost [%s]!\n", is, wholeTransTime)
						var record Record
						record = Record{
							Ip:         utils.NumToModel[kindOfModel],
							Type:       device[is].GetDeviceName(),
							Ptime:      strconv.FormatFloat(wholeInferTime, 'f', 2, 64),
							Ctime:      strconv.FormatFloat(wholeTransTime.Seconds(), 'f', 2, 64),
							Fps:        strconv.FormatFloat(fps, 'f', 2, 64),
							PowerWaste: strconv.FormatFloat(power, 'f', 2, 64),
							MemUsage:   "",
							Id: IdMessage,
							Batch:      strconv.Itoa(batchNum),
						}
						db.Table("record").Create(record)
						wholeRecord[is] = record

						ch <- is
					}(i)
				}
			}
			//等待所有协程执行结束
			for range ch {
				fmt.Println("ch now are: ", countDevice)
				countDevice--
				if countDevice == 0 {
					ch0 <- true
					ch1 <- true
					ch2 <- true
					ch3 <- true
					fmt.Println("ch complete")
					close(ch)
				}
			}

			if ErrorFlag == true {
				c.Next()
			}else {
				jsonRecord, _ := json.Marshal(wholeRecord)
				c.Data(http.StatusOK, "application/json", jsonRecord)
				c.Abort()
			}
			fmt.Println("Complete")
			fmt.Println("Over")

		}
	}
}

//记录查询
func (user *User) ApplyforRecord(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		type recordDate struct {
			Date string `json:"date"`
		}
		var date recordDate
		if err := c.ShouldBindJSON(&date); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
			return
		}
		if date.Date == "" {
			record := []RecordData{}
			jsonRecord, _ := json.Marshal(record)
			c.Data(http.StatusOK, "application/json", jsonRecord)
			c.Abort()
		} else {
			var res []RecordData
			db.Table("imgRecord").Where("time = ?", date.Date).Find(&res)
			for i := 0; i < len(res)/2; i++ {
				res[i], res[len(res)-i-1] = res[len(res)-i-1], res[i]
			}
			jsonRecord, _ := json.Marshal(res)


			c.Data(http.StatusOK, "application/json", jsonRecord)
			c.Abort()
		}

	}
}
