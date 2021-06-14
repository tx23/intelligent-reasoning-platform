package utils

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type HostSet struct {
	DeviceSet *[]OverDevice
}

var HostObj *HostSet
var MapHost map[string]OverDevice

func (hostObj *HostSet) Reload() {
	data, err := ioutil.ReadFile("config/hostSet.json")
	if err != nil {
		log.Fatal(err)
	}
	err = json.Unmarshal(data, &HostObj)
	if err != nil {
		log.Fatal(err)
	}

	MapHost = make(map[string]OverDevice)
	for _, v := range *HostObj.DeviceSet {
		MapHost[v.Devicename] = v
	}
}
