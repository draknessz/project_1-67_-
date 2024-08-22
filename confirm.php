<?php
// กำหนดข้อมูลเชื่อมต่อฐานข้อมูล MySQL
$servername = "localhost"; // เชื่อมต่อกับ localhost
$username = "ESP32"; // ชื่อผู้ใช้ MySQL
$password = "esp32io.com"; // รหัสผ่าน MySQL
$dbname = "db_esp32"; // ชื่อฐานข้อมูล MySQL

// สร้างการเชื่อมต่อ
$conn = new mysqli($servername, $username, $password, $dbname);

// ตรวจสอบการเชื่อมต่อ
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// รับข้อมูลจาก URL
$mode = isset($_GET['mode']) ? $_GET['mode'] : '';
$userID = isset($_GET['userID']) ? $_GET['userID'] : '';
$itemID = isset($_GET['itemID']) ? $_GET['itemID'] : '';

// ตรวจสอบว่ามีข้อมูลครบถ้วนหรือไม่
if (empty($mode) || empty($userID) || empty($itemID)) {
    echo "Error: Missing parameters";
    $conn->close();
    exit();
}

// ตรวจสอบว่า mode ถูกต้องหรือไม่
$valid_modes = ['1', '2'];
if (!in_array($mode, $valid_modes)) {
    echo "Error: Invalid mode";
    $conn->close();
    exit();
}

// แปลงค่า mode เป็น ENUM ที่ถูกต้อง
$mode_enum = ($mode == '1') ? 'borrow' : 'return';

// เตรียมคำสั่ง SQL สำหรับการบันทึกข้อมูล
$sql = "INSERT INTO history (mode, user_id, item_id, timestamp) VALUES (?, ?, ?, NOW())";

// เตรียมคำสั่ง SQL
$stmt = $conn->prepare($sql);

if ($stmt === false) {
    die("Prepare failed: " . $conn->error);
}

// ผูกค่าตัวแปรกับคำสั่ง SQL
$stmt->bind_param("sss", $mode_enum, $userID, $itemID);

// ดำเนินการคำสั่ง SQL
if ($stmt->execute()) {
    echo "Record inserted successfully";
} else {
    echo "Error: " . $stmt->error;
}

// ปิดการเชื่อมต่อฐานข้อมูล
$stmt->close();
$conn->close();
?>
