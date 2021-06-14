package utils

import (
	"io/ioutil"
	"strconv"
)

var ImageMap1 map[string]string
var ImageMap_Yaogan map[string]string

type ImageMap struct {
}

var ImageObj *ImageMap

func (imageMap *ImageMap) Reload() {
	ImageMap1 = make(map[string]string)
	ImageMap_Yaogan = make(map[string]string)

	files1, _ := ioutil.ReadDir("/home/archlab/intelligent_reasoning_system/intelligent_web/userimg/")
	//fmt.Println("Files: ", files1[0].Name())
	//var count string = "1"
	for k, v := range files1 {
		ImageMap1[strconv.Itoa(k+1)] = v.Name()
	}

	files2, _ := ioutil.ReadDir(PathObj.TestPath2)
	for k, v := range files2 {
		ImageMap_Yaogan[strconv.Itoa(k+1)] = v.Name()
	}

	//data, err := ioutil.ReadFile("config/DB.json")
	//if err != nil {
	//	log.Printf("ReadFile err!")
	//}
	//err = json.Unmarshal(data, &DBObj)
	//if err != nil {
	//	log.Printf("Json unmarshal err!")
	//}

}
