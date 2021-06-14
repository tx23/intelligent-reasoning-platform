package main

import (
	"encoding/json"
	"fmt"
	"irs/utils"
	"log"
	"net/http"

	sessions2 "github.com/gin-contrib/sessions"
	"github.com/gin-contrib/sessions/cookie"
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/mysql"

	//_ "irs/demo_face"
	"irs/demo_server"
	//"os/user"
)

type Data struct {
	IP          string `json:"ip"`
	Type        string `json:"type"`
	Ptime       string `json:"ptime"`
	Ctime       string `json:"ctime"`
	Power_waste string `json:"power_waste"`
	Mem         string `json:"mem"`
}

type ErrorRe struct {
	Name string `json:"name"`
}

type IdAndImageName struct {
	Id string `json:"id"`
	ImageName string `json:"imageName"`
}

func main() {
	utils.DBObj.Reload()
	utils.PathObj.Reload()
	utils.HostObj.Reload()
	utils.ModelObj.Reload()
	utils.ImageObj.Reload()

	fmt.Println(utils.MapHost)
	fmt.Println(utils.ModelMap)
	fmt.Println(utils.NumToModel)
	fmt.Println(utils.PathObj.TestPath, utils.PathObj.TestPath2)
	fmt.Println("ImageMap1: ", utils.ImageMap1)
	fmt.Println("ImageMap_Yaogan: ", utils.ImageMap_Yaogan)
	fmt.Println("------", utils.ImageMap1["2"])

	//db, err := gorm.Open("mysql", "root:123@tcp(47.101.58.106:3306)/smart_web?charset=utf8mb4&parseTime=True&loc=Local")
	db, err := gorm.Open(utils.DBObj.Property,
		utils.DBObj.UserName+":"+utils.DBObj.Password+"@tcp("+utils.DBObj.Host+":"+
			utils.DBObj.Port+")/"+utils.DBObj.Schemas+"?charset=utf8mb4&parseTime=True&loc=Local")

	if err != nil {
		fmt.Println("Connecting database failed!")
		log.Fatal(err)
	}

	//数据库自动迁移
	//db.AutoMigrate(&demo_server.User{})
	defer db.Close()

	//初始化LoginManager
	login := &demo_server.Login{}

	//路由组
	router := gin.Default()
	//router.Delims("{{", "}}")

	router.StaticFS("../css", http.Dir("./intelligent_web/css"))
	router.StaticFS("../js", http.Dir("./intelligent_web/js"))
	router.StaticFS("../img", http.Dir("./intelligent_web/img"))
	router.StaticFS("../userimg", http.Dir("./intelligent_web/userimg"))
	router.LoadHTMLFiles("./intelligent_web/login.html",
		"./intelligent_web/index.html",
		"./intelligent_web/performance_monitor.html",
		"./intelligent_web/record_query.html",
		"./intelligent_web/device_manager.html")
	//router.LoadHTMLGlob("./intelligent_web/login.html")

	store := cookie.NewStore([]byte("secret"))
	router.Use(sessions2.Sessions("mysessions", store))

	router.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "login.html", nil)
	})

	/*
		Test1：用户管理
		Test2: 用户-设备映射
	*/
	//user := demo_server.NewUser("", "")
	user := demo_server.User{}
	routerGroup1 := router.Group("/user")
	{
		routerGroup1.StaticFS("/css", http.Dir("./intelligent_web/css"))
		routerGroup1.StaticFS("/js", http.Dir("./intelligent_web/js"))
		routerGroup1.StaticFS("/img", http.Dir("./intelligent_web/img"))
		routerGroup1.StaticFS("/userimg", http.Dir("./intelligent_web/userimg"))
		routerGroup1.StaticFS("/record", http.Dir("./intelligent_web/record"))

		routerGroup1.Use(sessions2.Sessions("mysessions", store))
		//Test1：用户管理
		routerGroup1.POST("/loginIn", login.LoginIn(db), func(c *gin.Context) {
			//c.HTML(301, "index.html", nil)
			username, _ := c.Get("username")
			password, _ := c.Get("password")

			fmt.Println("After loginIn, username set to context is: ", username.(string))
			fmt.Println("After loginIn, password set to context is: ", password.(string))

			user = demo_server.User{Username: username.(string), Password: password.(string)}
			//c.Redirect(http.StatusTemporaryRedirect, "http://www.baidu.com")
			//c.HTML(http.StatusOK, "index.html", nil)
		})

		routerGroup1.POST("/loginUp", login.LoginUp(db))

		//Test2：用户-设备映射
		routerGroup1.GET("/deviceManager", func(c *gin.Context) {
			c.HTML(http.StatusOK, "performance_monitor.html", nil)
		})
		routerGroup1.POST("/applyDevice", func(c *gin.Context) {
			fmt.Println("User ", user, " is applying for device!")
			c.Next()
		}, user.ApplyForDevice(db), func(c *gin.Context) {
			fmt.Println("flag: ", demo_server.LoginFlag)
			if demo_server.LoginFlag == true {
				fmt.Println("Enter if")
				demo_server.LoginFlag = false
				c.HTML(http.StatusMovedPermanently, "index.html", nil)
				c.Abort()
			}else {
				c.Next()
			}
		}, user.GetDeviceRecord(db))

		routerGroup1.POST("/removeDevice", func(c *gin.Context) {
			fmt.Println("User ", user, " is removing for device!")
			c.Next()
		}, user.RemoveDevice(db), user.GetDeviceRecord(db))

		routerGroup1.POST("/getDeviceData", func(c *gin.Context) {
			fmt.Println("User ", user, " is listing for device!")
			c.Next()
		}, user.ListDevice(db), user.GetDeviceRecord(db))

		//跳转首页
		routerGroup1.GET("/jumpIndex", func(c *gin.Context) {
			c.HTML(http.StatusOK, "index.html", nil)
		})
		// 跳转性能测试页面
		routerGroup1.GET("/jumpPerformance", func(c *gin.Context) {
			c.HTML(http.StatusOK, "performance_monitor.html", nil)
		})
		//跳转记录查询页面
		routerGroup1.GET("/jumpRecord", func(c *gin.Context) {
			c.HTML(http.StatusOK, "record_query.html", nil)
		})
		//跳转设备管理页面
		routerGroup1.GET("/jumpDevice", func(c *gin.Context) {
			c.HTML(http.StatusOK, "device_manager.html", nil)
		})

		//性能测试初始化
		routerGroup1.POST("/performanceOrigin", user.PerformanceOrigin(db))

		//历史数据初始化
		routerGroup1.POST("/historyOrigin", user.HistoryOrigin(db))

		//
		routerGroup1.POST("/historyMoreDetails", func(context *gin.Context) {
			fmt.Println("开始执行")

			type HistoryId struct {
				HistoryDetails string `json:"historyDetails"`
			}
			var hId HistoryId;
			if err := context.ShouldBindJSON(&hId); err != nil {
				fmt.Println("bind json err")
				fmt.Println(err)
				context.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
				return
			}
			id:=hId.HistoryDetails;
			fmt.Println("xiamianshizifuchuan")
			fmt.Println(id)
			var imageToRecordList []demo_server.ImageToRecord
			db.Table("imageToRecord").Where("id=?",id).Find(&imageToRecordList)

			fmt.Println("开始执行2")
			jsonImageList,_:=json.Marshal(imageToRecordList)
			fmt.Println(jsonImageList)
			context.Data(http.StatusOK, "application/json", jsonImageList)
		})
		//性能测试请求
		routerGroup1.POST("/applyTest", user.ApplyForTest(db), func(c *gin.Context) {
			fmt.Println("Enter Test Next")
			//data := []Data{
			//	//{IP: "192.168.3.11", Type: "NVIDIA Jetson AGX Xavier", Ctime: "1.0s", Ptime: "0.6s", Power_waste: "", Mem: "50"},
			//	{IP: "192.168.3.13", Type: "NVIDIA Jetson TX2", Ctime: "1.0s", Ptime: "0.6s", Power_waste: "", Mem: "50"},
			//}
			//jsonData, _ := json.Marshal(data)
			var errorRe ErrorRe
			if demo_server.ErrorFlag == true {
				errorRe = ErrorRe{
					Name: "err",
				}
				demo_server.ErrorFlag = false
			}
			jsonError, _ := json.Marshal(errorRe)
			fmt.Println(string(jsonError))
			//result := strings.Replace(string(jsonData), "\"", "\\\"", -1)
			//fmt.Println(data)
			//fmt.Println(string(jsonData))
			//result := "hahahaha"
			//result := "hahaha"
			//fmt.Println(result)
			//c.HTML(http.StatusOK, "performance_monitor.html", nil)
			//c.JSON(http.StatusOK, gin.H{
			//	"name": string(jsonData),
			//})
			//c.JSON(http.StatusOK, jsonData)
			c.Data(http.StatusOK, "application/json", jsonError)
			//c.HTML(http.StatusOK, "performance_monitor.html", nil)
			fmt.Println("User ", user, " is apply for testing!")
			//c.Next()
		})

		//记录查询请求
		routerGroup1.POST("/record", user.ApplyforRecord(db))

		routerGroup1.Any("/applyForTest1", func(c *gin.Context) {
			c.HTML(http.StatusOK, "record_query.html", nil)
		})

	}

	/*
		Test：设备管理
	*/

	//初始化DeviceManager
	// deviceManager := demo_server.NewDeviceManager()
	// routerGroup2 := router.Group("/manager")
	// {
	// routerGroup2.POST("/removeDevice", func(c *gin.Context) {
	// 	device1 := Device{IP: "192.168.3.11", Type: "NVIDIA Jetson AGX Xavier", Time: "2020-11-19", Status: "已连接"}
	// 	device2 := Device{IP: "192.168.3.13", Type: "NVIDIA Jetson AGX Xavier", Time: "2020-11-19", Status: "已连接"}
	// 	device := make([]Device, 2)
	// 	device[0] = device1
	// 	device[1] = device2
	// 	dataDevice := DataDevice{
	// 		Available: device,
	// 		Compare:   device,
	// 	}
	// 	jsonData, _ := json.Marshal(dataDevice)
	// 	fmt.Println(string(jsonData))
	// 	c.Data(http.StatusOK, "application/json", jsonData)
	// 	fmt.Println("Remove Device!")
	// 	c.Next()
	// })
	// routerGroup2.POST("/getDeviceData", deviceManager.GetDeviceData(db))
	// 	routerGroup2.POST("/addDevice", deviceManager.AddDevice(db))
	// 	routerGroup2.POST("/removeDevice", deviceManager.RemoveDevice(db))
	// }

	_ = router.Run(":9090")

}

//记录查询
