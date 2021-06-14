package demo_server

import (
	"fmt"
	"net/http"

	sessions2 "github.com/gin-contrib/sessions"

	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
)

type Login struct{}

func (login *Login) LoginIn(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		username := c.PostForm("username")
		password := c.PostForm("password")
		user := NewUser(username, password)
		u := user.FindUser(db)
		fmt.Println("user=", user)
		fmt.Println(u)
		if u == nil {
			fmt.Printf("Login failed!\n")
			c.HTML(http.StatusOK, "login.html", nil)
		}
		fmt.Println(u.GetPassWord())
		if u != nil && password == u.GetPassWord() {
			fmt.Println("Test session1") //Test

			session := sessions2.Default(c)
			fmt.Println("GetSession: ", session.Get(username))
			if session.Get(username) == nil {
				//设置session
				session.Set(username, username)
				//保存session
				_ = session.Save()

				fmt.Println("Save session: ", session.Get(username))
			}
			c.Set("username", user.GetUserName())
			c.Set("password", user.GetPassWord())

			fmt.Printf("Login successfully!\n")
			c.HTML(http.StatusOK, "index.html", nil)
			c.Next()
		} else {
			fmt.Printf("Login failed!\n")
			c.HTML(http.StatusOK, "login.html", nil)
		}
	}
}

func (login *Login) LoginUp(db *gorm.DB) gin.HandlerFunc {
	return func(c *gin.Context) {
		fmt.Println("Enter LoginUp!")

		username := c.PostForm("username")
		password := c.PostForm("password")
		user := NewUser(username, password)

		fmt.Printf("username=%s, password=%s\n", username, password)
		fmt.Println("user=", user)

		user.InsertUser(db)
	}
}

func (login *Login) isLogined(doCheck bool) gin.HandlerFunc {
	return func(c *gin.Context) {
		if doCheck {
			c.Next()
		} else {
			c.Redirect(http.StatusMovedPermanently, "http://www.sogo.com")
		}
	}
}
