from PIL import Image
import os

path = "testImgWeixin/"
for i in os.listdir("./testImgWeixin"):
    image = Image.open(path+i)
    image.save("weixing/"+i.split('.')[0]+".jpg")

    
