<?php
// กำหนดข้อมูลเชื่อมต่อฐานข้อมูล MySQL
$servername = "localhost"; // เชื่อมต่อกับ localhost
$username = "ESP32"; // ชื่อผู้ใช้ MySQL
$password = "esp32io.com"; // รหัสผ่าน MySQL
$dbname = "db_esp32"; // ชื่อฐานข้อมูล MySQL

// สร้างการเชื่อมต่อ
$conn = new mysqli($servername, $username, $password, $dbname);

// รับค่า userID จาก URL parameter
if (isset($_GET['UID'])) {
    $UID = $_GET['UID'];

    // สร้างการเชื่อมต่อ
    $conn = new mysqli($servername, $username, $password, $dbname);

    // ตรวจสอบการเชื่อมต่อ
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    // สร้างคำสั่ง SQL เพื่อค้นหาข้อมูลจากฐานข้อมูล
    $sql = "SELECT * FROM users WHERE  users_id = '$UID'";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        // หากพบข้อมูล สร้าง associative array เพื่อเก็บข้อมูล
        $row = $result->fetch_assoc();

        // สร้างข้อมูล JSON ที่จะส่งกลับไปยัง ESP32
        $response = array(
                'id' => $row['id'],
                'usersid' => $row['users_id'],
                'fname' => $row['fname'],
                // เพิ่มฟิลด์ตามต้องการ
        );
    } else {
        // หากไม่พบข้อมูล
        $response = array(
            'success' => false,
            'message' => 'Data not found'
        );
    }

    // ปิดการเชื่อมต่อ MySQL
    $conn->close();

} else {
    // หากไม่มีค่า QRID ส่งมา
    $response = array(
        'success' => false,
        'message' => 'QRID parameter is missing'
    );
}

// ส่งข้อมูลกลับในรูปแบบ JSON
header('Content-Type: application/json');
echo json_encode($response);
?>




