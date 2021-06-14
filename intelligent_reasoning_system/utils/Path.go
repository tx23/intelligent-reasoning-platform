package utils

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type Path struct {
	TestPath   string
	TestPath2  string
	RecordPath string
}

var PathObj *Path

func (path *Path) Reload() {
	data, err := ioutil.ReadFile("config/path.json")
	if err != nil {
		log.Printf("ReadFile err!")
	}
	err = json.Unmarshal(data, &PathObj)
	if err != nil {
		log.Printf("Json unmarshal err!")
	}
}
