package demo_face

import (
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
)

type ILogin interface {
	LoginIn(db *gorm.DB) gin.HandlerFunc // 登录
	LoginUp(db *gorm.DB)  gin.HandlerFunc//注册
	isLogined(doCheck bool) gin.HandlerFunc //是否已登录
}
