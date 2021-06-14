package utils

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type Model struct {
	ModelName string
	Port      string
	Num 	  string
}

type ModelSet struct {
	ModelAll *[]Model
}

var ModelObj *ModelSet
var ModelMap map[string]string
var NumToModel map[string]string

func (model *ModelSet) Reload() {
	data, err := ioutil.ReadFile("config/model.json")
	if err != nil {
		log.Fatal(err)
	}
	err = json.Unmarshal(data, &ModelObj)
	if err != nil {
		log.Fatal(err)
	}

	ModelMap = make(map[string]string)
	for _, v := range *ModelObj.ModelAll {
		ModelMap[v.ModelName] = v.Port
	}

	NumToModel = make(map[string]string)
	for _, v := range *ModelObj.ModelAll {
		NumToModel[v.Num] = v.ModelName
	}
}
