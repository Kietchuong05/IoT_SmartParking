# Tiểu luận môn Lập trình IoT: 
- Đề tài: Bãi đỗ xe thông minh
- Platform: Homeassistant
## Các thành viên làm tiểu luận:
- Lê Thành Đạt - 23050041
- Chướng Và Kiệt - 23050043
## Hướng dẫn sử dụng:
Phần mềm cần thiết để chạy tiểu luận:
- Docker Desktop.
- Arduino IDE. (Để nạp code cho ESP8266).
## Triển khai Homeassistant
- Hãy mở ứng dụng Docker của bạn.
- Mở CMD và chạy dòng lệnh sau:
```sh
docker pull homeassistant/home-assistant
 ```
- Sau khi chạy mã, bạn sẽ nhận được kết quả sau:
![image](https://github.com/user-attachments/assets/d1268bf6-c06e-4787-ad65-37901af2e163)

- Tạo thư mục tên homeassistant
- Mở CMD tại thư mục này.
- Nhập dòng lệnh sau: 
```sh
 docker run -d --name="homeassistant" -v E:\homeassistant:/config -p 8123:8123 homeassistant/home-assistant:latest
```
##### Lưu ý: thư mục homeassistant đang ổ địa E hay sửa lại nếu bạn đang lưu ổ địa khác.
- Sau khi chạy xong dòng lệnh trên bạn hãy vào trình duyệt bất kì và tìm kiếm `localhost:8123`, nó sẽ hiện trang Homeassistant
- ![image](https://github.com/user-attachments/assets/459b4a2b-b84b-4cdb-8e1f-7ac2ad6c91f6)

