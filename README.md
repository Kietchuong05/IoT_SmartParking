# Tiểu luận môn Lập trình IoT: 
- Đề tài: Bãi đỗ xe thông minh
- Platform: Homeassistant
## Các thành viên làm tiểu luận:
- Lê Thành Đạt
- Chướng Và Kiệt
## Hướng dẫn sử dụng:
Phần mềm cần thiết để chạy tiểu luận:
- Docker Desktop.
- Arduino IDE. (Để nạp code cho ESP8266).
## Triển khai Homeassistant
- Tạo thư mục tên homeassistant
- Mở CMD, Git Bash, hoặc Terminal tại thư mục này.
- Nhập dòng lệnh sau: 
```sh
 docker run -d --name="homeassistant" -v E:\homeassistant:/config -p 8123:8123 homeassistant/home-assistant:latest
```
### Lưu ý: thư mục homeassistant đang ổ địa E hay sửa lại nếu bạn đang lưu ổ địa khác.
