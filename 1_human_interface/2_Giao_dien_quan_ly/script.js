// ------------------------Nút chuyển TAB---------------------//
function buttonToolbar() {
  const homeBtn = document.getElementById("homeBtn");
  const controlBtn = document.getElementById("controlBtn");
  const workSpace1 = document.querySelector(".workSpace1");
  const workSpace2 = document.querySelector(".workSpace2");

  homeBtn.addEventListener("click", () => {
    console.log("ấn homeBtn");
    workSpace1.style.display = "block";
    workSpace2.style.display = "none";
  });

  controlBtn.addEventListener("click", () => {
    console.log("ấn controlBtn");
    workSpace1.style.display = "none";
    workSpace2.style.display = "block";
  });
}
document.addEventListener("DOMContentLoaded", () => {
  buttonToolbar(); // Gọi hàm khi trang đã sẵn sàng
});
//
//
//
// ----------------------FireBase-----------------------//
// Cấu hình Firebase với thông tin cần thiết
const firebaseConfig = {
  apiKey: "AIzaSyACGGsUd4RqknExecIhgx57_xv7A5n_7v8", // Khóa API của Firebase
  authDomain: "web-firebase-test-64f79.firebaseapp.com", // Tên miền xác thực của ứng dụng
  databaseURL: "https://web-firebase-test-64f79-default-rtdb.firebaseio.com", // URL của cơ sở dữ liệu
  projectId: "web-firebase-test-64f79", // ID của dự án Firebase
  storageBucket: "web-firebase-test-64f79.appspot.com", // Bucket lưu trữ của Firebase
  messagingSenderId: "229211403998", // ID của người gửi tin nhắn
  appId: "1:229211403998:web:2b220c676f056353b2b44f", // ID của ứng dụng Firebase
};

// Khởi tạo ứng dụng Firebase với cấu hình trên
firebase.initializeApp(firebaseConfig); // Khởi tạo ứng dụng Firebase
const database = firebase.database(); // Tạo tham chiếu đến cơ sở dữ liệu Firebase

// Lưu trữ danh sách các bàn đã hiển thị trước đó để phát hiện thay đổi
let previousTables = {};
// Lưu trạng thái highlight của từng bàn
let highlightStatus = {};

// Hàm để hiển thị danh sách các bàn
function hienThiDanhSachTable(tablesData) {
  const tableListDiv = document.getElementById("table-list"); // Tham chiếu đến phần tử danh sách bàn
  tableListDiv.innerHTML = ""; // Xóa nội dung trước đó

  // Lặp qua từng bàn trong danh sách
  for (const tableName in tablesData) {
    const tableData = tablesData[tableName]; // Dữ liệu của bàn hiện tại
    const tableItemDiv = document.createElement("div"); // Tạo một div cho bàn
    tableItemDiv.classList.add("table-item"); // Thêm lớp CSS cho div

    tableItemDiv.innerHTML = `
        <strong>${tableName} - Tổng tiền: ${tableData.totalAmount.toLocaleString(
      "de-DE"
    )} đ</strong>
        <div class="timestamp">Thời gian đặt: ${new Date(
          tableData.timestamp
        ).toLocaleString()}</div>
      `; // Thiết lập nội dung HTML cho div

    // Thêm highlight nếu bàn mới hoặc dữ liệu đã thay đổi
    if (
      !previousTables[tableName] || // Nếu bàn chưa tồn tại trong previousTables
      //Sử dụng JSON.stringify để so sánh danh sách đơn hàng của bàn hiện tại với danh sách đơn hàng trước đó.
      //Nếu chúng khác nhau, bàn sẽ được highlight.
      JSON.stringify(previousTables[tableName]) !== JSON.stringify(tableData) // So sánh dữ liệu
    ) {
      highlightStatus[tableName] = true; // Đặt trạng thái highlight thành true
    }

    if (highlightStatus[tableName]) {
      tableItemDiv.classList.add("highlight");
    }

    // Sự kiện click để hiển thị chi tiết đơn hàng và xóa highlight
    tableItemDiv.addEventListener("click", function () {
      hienThiChiTietDonHang(tableName, tableData); // Hiển thị chi tiết đơn hàng
      tableItemDiv.classList.remove("highlight"); // Xóa highlight khi click vào bàn
      highlightStatus[tableName] = false; // Cập nhật trạng thái
    });

    // Thêm div bàn vào danh sách
    tableListDiv.appendChild(tableItemDiv);
    // console.log(tableName);
    // console.log(tablesData);
  }

  // Cập nhật lại danh sách các bàn đã hiển thị trước đó
  previousTables = tablesData; // Cập nhật previousTables
}

// Hàm để hiển thị chi tiết đơn hàng của một bàn
function hienThiChiTietDonHang(tableName, tableData) {
  const orderListDiv = document.getElementById("order-list");
  const orderDetailsDiv = document.getElementById("order-details");
  const totalAmountSpan = document.getElementById("total-amount");

  orderListDiv.style.display = "block"; // Hiển thị chi tiết đơn hàng
  orderDetailsDiv.innerHTML = `<h3>Đơn hàng của ${tableName}</h3>`; // Thiết lập tiêu đề cho chi tiết đơn hàng

  // Hiển thị các món ăn đã đặt
  tableData.orders.forEach((order) => {
    const orderDiv = document.createElement("div"); // Tạo div cho mỗi món ăn
    orderDiv.classList.add("cacMon");
    var tienCacMon = order.price * order.quantity;
    orderDiv.innerHTML = `<h4>${order.name}</h4> - Số lượng: ${
      order.quantity
    }, Giá: ${tienCacMon.toLocaleString("de-DE")} đ`; // Thiết lập nội dung cho món ăn

    orderDetailsDiv.appendChild(orderDiv); // Thêm div món ăn vào danh sách
  });

  totalAmountSpan.textContent = tableData.totalAmount.toLocaleString("de-DE"); // Cập nhật tổng tiền

  // Sự kiện click cho nút reset thông tin của một bàn
  document.getElementById("clear-table").onclick = function () {
    clearTableData(tableName); // Gọi hàm reset cho bàn cụ thể
  };
}

// Lấy dữ liệu từ Firebase
function fetchTableData() {
  const purchasesRef = database.ref("Purchases"); // Tham chiếu đến node "Purchases" trong cơ sở dữ liệu
  purchasesRef.on("value", (snapshot) => {
    // Lắng nghe sự kiện thay đổi dữ liệu
    const data = snapshot.val(); // Lấy dữ liệu từ snapshot
    if (data) {
      hienThiDanhSachTable(data); // Hiển thị danh sách bàn nếu có dữ liệu
    } else {
      document.getElementById("table-list").innerHTML =
        "Không có bàn nào đang đặt hàng."; // Thông báo nếu không có bàn
    }
  });
}

// Hàm để xóa dữ liệu của một bàn cụ thể từ Firebase
function clearTableData(tableName) {
  const tableRef = database.ref(`Purchases/${tableName}`); // Tham chiếu đến bàn cụ thể
  tableRef
    .set(null) // Đặt giá trị của bàn thành null để xóa dữ liệu
    .then(() => {
      alert(`Đã xóa thông tin bàn ${tableName} thành công!`); // Thông báo thành công
      fetchTableData(); // Cập nhật danh sách bàn
    })
    .catch((error) => {
      console.error("Lỗi khi xóa thông tin bàn: ", error); // Xử lý lỗi nếu có
    });
}

// Hàm để xóa tất cả dữ liệu bàn từ Firebase
function resetAllTables() {
  const purchasesRef = database.ref("Purchases"); // Tham chiếu đến node "Purchases"
  purchasesRef
    .set(null) // Đặt giá trị của node thành null để xóa toàn bộ dữ liệu
    .then(() => {
      alert("Đã xóa tất cả thông tin bàn thành công!"); // Thông báo thành công
      fetchTableData(); // Cập nhật danh sách bàn
    })
    .catch((error) => {
      console.error("Lỗi khi xóa tất cả thông tin bàn: ", error); // Xử lý lỗi nếu có
    });
}

// Gọi hàm để lấy dữ liệu khi trang web được load
fetchTableData(); // Khởi động quá trình lấy dữ liệu từ Firebase

// Sự kiện click cho nút reset tất cả thông tin
document.getElementById("clear-all").addEventListener("click", resetAllTables); // Gán sự kiện click cho nút reset tất cả thông tin
//
//
//
// ----------------------webSoket-----------------------//

let socket; // Khai báo biến socket để lưu kết nối WebSocket
let bepStatus;
let banDaGui;

document.addEventListener("DOMContentLoaded", function () {
  // Kết nối WebSocket tới ESP32
  // socket = new WebSocket("ws://192.168.0.200/ws");             // wifi nhà

  socket = new WebSocket("ws://192.168.146.200/ws"); // wifi điện thoại

  socket.onopen = function () {
    console.log("Đã kết nối WebSocket với ESP32");
  };

  socket.onmessage = function (event) {
    const receivedData = event.data;
    console.log("Nhận được tin nhắn từ ESP32: " + event.data);

    // ví dụ: receivedData = "ban so 3"
    // -> dữ liệu của tableNumberMatch = {"ban so 3", 3}
    // Khi sử dụng match() với một biểu thức chính quy có dấu ngoặc ():
    // Kết quả trả về là một mảng nếu có chuỗi khớp.
    // Cấu trúc mảng:
    // [0]: Chuỗi khớp toàn bộ.
    // [1]: Kết quả khớp với nhóm con đầu tiên (phần nằm trong ()).

    const tableNumberMatch = receivedData.match(/ban so (\d+)/);
    if (tableNumberMatch) {
      const soBan = tableNumberMatch[1];
      console.log("Cập nhật trạng thái cho bàn: " + soBan);
      // khi đến bàn cần đến thì "bếp" sáng thể hiện đang đi về
      if (banDaGui === soBan) {
        bepStatus = 0; // bếp sáng
      }
      updateButtonState(soBan, false); // Tắt sáng nút khi nhận tín hiệu
    }
  };

  socket.onclose = function () {
    console.log("WebSocket đã đóng");
  };

  socket.onerror = function (error) {
    console.error("Lỗi WebSocket: " + error);
  };
});

// Hàm gửi số bàn tới ESP32
function sendTableNumber(soBan) {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(soBan.toString());
    banDaGui = soBan.toString(); //gán bàn gửi để tiện so sánh xem đến đích chưa
    bepStatus = 1; // bếp tối
    console.log("Đã gửi số bàn: " + soBan);
    updateButtonState(soBan, true); // Bật sáng nút khi gửi
  } else {
    console.log("WebSocket chưa kết nối.");
  }
}

// Cập nhật trạng thái của nút
function updateButtonState(soBan, isActive) {
  const btns = document.querySelectorAll(".workSpace2 .tableAll button");
  const bep = document.getElementById("button0");
  btns.forEach((btn) => {
    //Nếu nội dung hiển thị trên nút khớp với chuỗi "Bàn {soBan}"
    if (btn.textContent === `Bàn ${soBan}`) {
      if (isActive) {
        btn.style.backgroundColor = "rgb(69, 209, 111)";
        btn.style.boxShadow = "0px 0px 20px rgb(69, 209, 111)";
      } else {
        btn.style.backgroundColor = "salmon";
        btn.style.boxShadow = "0px 0px 10px salmon";
      }
    }
  });
  // điều kiện đi về bếp
  if (bepStatus === 0) {
    bep.style.backgroundColor = "rgb(69, 209, 111)";
    bep.style.boxShadow = "0px 0px 20px rgb(69, 209, 111)";
  } else {
    bep.style.backgroundColor = "salmon";
    bep.style.boxShadow = "0px 0px 10px salmon";
  }
}
