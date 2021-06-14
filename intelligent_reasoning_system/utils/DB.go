package utils

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type DB struct {
	Property string
	Host     string
	Port     string
	UserName string
	Password string
	Schemas  string
}

var DBObj *DB

func (db *DB) Reload() {
	data, err := ioutil.ReadFile("config/DB.json")
	if err != nil {
		log.Printf("ReadFile err!")
	}
	err = json.Unmarshal(data, &DBObj)
	if err != nil {
		log.Printf("Json unmarshal err!")
	}
}

func init() {
	DBObj = &DB{
		Property: "mysql",
		Host:     "47.101.58.106",
		Port:     "3306",
		UserName: "root",
		Password: "123",
		Schemas:  "smart_web",
	}
	DBObj.Reload()
}
