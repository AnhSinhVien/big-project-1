const firebaseConfig = {
  apiKey: "AIzaSyACGGsUd4RqknExecIhgx57_xv7A5n_7v8",
  authDomain: "web-firebase-test-64f79.firebaseapp.com",
  databaseURL: "https://web-firebase-test-64f79-default-rtdb.firebaseio.com",
  projectId: "web-firebase-test-64f79",
  storageBucket: "web-firebase-test-64f79.appspot.com",
  messagingSenderId: "229211403998",
  appId: "1:229211403998:web:2b220c676f056353b2b44f",
};
firebase.initializeApp(firebaseConfig);
var database = firebase.database();

// Lấy tất cả các thẻ có class 'table' bên trong .all-table
const tables = document.querySelectorAll(".all-table .table");

// Gán sự kiện click cho mỗi thẻ
tables.forEach((table) => {
  table.addEventListener("click", function () {
    // Với trường hợp chọn nhiều bàn
    if (this.classList.contains("clicked")) {
      this.classList.remove("clicked");
      this.style.backgroundColor = "wheat";
      console.log(this.textContent + " đã hủy chọn");
    } else {
      this.classList.add("clicked");
      this.style.backgroundColor = "yellow";
      console.log(this.textContent + " đã được chọn");
    }

    // Cập nhật giá trị `data-value` cho tất cả các thẻ
    tables.forEach((table) => {
      table.dataset.value = table.classList.contains("clicked") ? 1 : 0;
    });
  });
});

// *************phần Giỏ hàng****************//
const btn = document.querySelectorAll(".product-items button");
btn.forEach(function (button) {
  button.addEventListener("click", function (event) {
    // lấy đối tượng nút vừa được bấm.
    var btnItem = event.target;
    //lấy phần tử cha của nút, có thể là thẻ chứa thông tin sản phẩm.
    var product = btnItem.parentElement;
    var productImg = product.querySelector("img").src;
    var productName = product.querySelector("h1").innerText;
    var productPrice = product.querySelector("span").innerText;

    // Thêm sản phẩm vào giỏ hàng
    addCart(productImg, productName, productPrice);
  });
});

function addCart(productImg, productName, productPrice) {
  var addTr = document.createElement("tr");
  var cartItem = document.querySelectorAll("tbody tr");

  // Kiểm tra nếu sản phẩm đã có trong giỏ hàng
  for (var i = 0; i < cartItem.length; i++) {
    var productT = cartItem[i].querySelector(".title1");
    if (productT.innerHTML === productName) {
      alert("Sản phẩm của bạn đã có trong giỏ hàng");
      return;
    }
  }

  // Gắn thẻ Tr mới
  var trContent =
    '<tr><td><img src="' +
    productImg +
    '" alt="" /> <span class="title1">' +
    productName +
    '</span></td><td><p><span class="price">' +
    productPrice +
    '</span><sup>đ</sup></p></td><td><input type="number" value="1" min="1" class="quantity-input" /></td><td class="delete-btn">Xoá</td></tr>';

  addTr.innerHTML = trContent;
  var cartTable = document.querySelector("tbody");
  cartTable.append(addTr);

  // Cập nhật tổng tiền khi số lượng thay đổi
  addTr.querySelector(".quantity-input").addEventListener("input", function () {
    if (this.value < 1) this.value = 1; // Đảm bảo số lượng không nhỏ hơn 1
    cartTotal(); // Tính lại tổng tiền
  });

  // Sự kiện xóa sản phẩm
  addTr.querySelector(".delete-btn").addEventListener("click", function () {
    this.parentElement.remove();
    cartTotal(); // Cập nhật lại tổng tiền khi xóa sản phẩm
  });

  cartTotal();
}

// Tính tổng số tiền
function cartTotal() {
  var cartItem = document.querySelectorAll("tbody tr");
  var sumLast = 0;

  for (var i = 0; i < cartItem.length; i++) {
    var inputValue = cartItem[i].querySelector("input").value;
    var productPrice = cartItem[i].querySelector(".price").innerHTML;

    // Loại bỏ dấu chấm và chuyển giá trị thành số
    productPrice = productPrice.replace(/\./g, "");
    var sumTotal = inputValue * parseFloat(productPrice);
    sumLast += sumTotal;
  }

  var cartTotalA = document.querySelector(".price-total span");
  cartTotalA.innerHTML = sumLast.toLocaleString("de-DE");
  var cartPrite = document.querySelector(".cart-icon span");
  cartPrite.innerHTML = sumLast.toLocaleString("de-DE");
}

// Lấy thông tin bàn và đơn hàng khi nhấn nút "Mua hàng"
const purchaseButton = document.querySelector(".cart button");
purchaseButton.addEventListener("click", function () {
  // Lấy các bàn được chọn
  const selectedTables = document.querySelectorAll(".all-table .table.clicked");
  // Kiểm tra nếu chưa có bàn nào được chọn
  if (selectedTables.length === 0) {
    alert("Vui lòng chọn bàn trước khi thực hiện mua hàng.");
    return; // Dừng việc thực hiện nếu không có bàn nào được chọn
  }

  selectedTables.forEach((table) => {
    const tableName = table.textContent.trim();

    // Lấy thông tin các đơn hàng trong giỏ hàng
    const cartItems = [];
    const cartRows = document.querySelectorAll("tbody tr");
    cartRows.forEach((row) => {
      const productName = row.querySelector(".title1").textContent;
      const productPrice = row
        .querySelector(".price")
        .textContent.replace(/\./g, "");
      const quantity = row.querySelector(".quantity-input").value;
      cartItems.push({
        name: productName,
        price: parseFloat(productPrice),
        quantity: parseInt(quantity, 10),
      });
    });

    // Tính tổng số tiền
    const totalAmount = parseFloat(
      document.querySelector(".price-total span").textContent.replace(/\./g, "")
    );

    // Tạo đối tượng dữ liệu để gửi lên Firebase
    const purchaseData = {
      orders: cartItems,
      totalAmount: totalAmount,
      timestamp: new Date().toISOString(), // Thêm thời gian đặt hàng
    };

    // Gửi dữ liệu lên Firebase với tên bàn là khóa chính
    var firebaseRef = firebase
      .database()
      .ref()
      .child("Purchases")
      .child(tableName);
    firebaseRef
      .set(purchaseData)
      .then(() => {
        console.log("Dữ liệu đã được gửi lên Firebase thành công.");
        // Có thể thêm mã để thông báo cho người dùng hoặc làm gì đó khác ở đây
      })
      .catch((error) => {
        console.error("Lỗi khi gửi dữ liệu lên Firebase: ", error);
      });
  });
});

// làm hiệu ứng mở ra đóng vào cho cart
const cartClose = document.querySelector(".fa-xmark");
const cartShow = document.querySelector(".fa-cart-shopping");

// mở
cartShow.addEventListener("click", function () {
  document.querySelector(".cart").style.right = 0;
});
// đóng
cartClose.addEventListener("click", function () {
  document.querySelector(".cart").style.right = "-100%";
});