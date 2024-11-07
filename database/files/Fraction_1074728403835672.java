
package PhanSo;

/**
 *
 * @author Admin
 */
import java.util.Arrays;
import java.util.Random;
import java.util.Scanner;

class PhanSo {
    private int tuSo;
    private int mauSo;
    public PhanSo(int tuSo, int mauSo) {
        this.tuSo = tuSo;
        if (mauSo != 0)
            this.mauSo = mauSo;
         else
            System.out.println("Mau so khong the bang 0.");
    }

    // Cộng hai phân số
    public PhanSo cong(PhanSo ps) {
        int tuSoMoi = this.tuSo * ps.mauSo + ps.tuSo * this.mauSo;
        int mauSoMoi = this.mauSo * ps.mauSo;
        return new PhanSo(tuSoMoi, mauSoMoi).donGian();
    }

    // Trừ hai phân số
    public PhanSo tru(PhanSo ps) {
        int tuSoMoi = this.tuSo * ps.mauSo - ps.tuSo * this.mauSo;
        int mauSoMoi = this.mauSo * ps.mauSo;
        return new PhanSo(tuSoMoi, mauSoMoi).donGian();
    }

    // Nhân hai phân số
    public PhanSo nhan(PhanSo ps) {
        int tuSoMoi = this.tuSo * ps.tuSo;
        int mauSoMoi = this.mauSo * ps.mauSo;
        return new PhanSo(tuSoMoi, mauSoMoi).donGian();
    }

    // Chia hai phân số
    public PhanSo chia(PhanSo ps) {
        if (ps.tuSo != 0) {
            return this.nhan(ps.nghichDao());
        } else {
            System.out.println("Khong the chia cho 0.");
            // Có thể thêm mã lệnh để xử lý lỗi khác tùy ý.
            return null;
        }
    }

    // Nghịch đảo phân số
    public PhanSo nghichDao() {
        return new PhanSo(this.mauSo, this.tuSo).donGian();
    }

    // Đơn giản hóa phân số
    public PhanSo donGian() {
        int ucln = timUCLN(Math.abs(this.tuSo), Math.abs(this.mauSo));
        return new PhanSo(this.tuSo / ucln, this.mauSo / ucln);
    }

    public int getTuSo() {
        return tuSo;
    }

    public int getMauSo() {
        return mauSo;
    }
    
    // Tìm ước chung lớn nhất
    private int timUCLN(int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return Math.abs(a);
    }

    // Chuyển đổi phân số thành chuỗi
    public String toString() {
        return tuSo + "/" + mauSo;
    }

    public int compareTo(PhanSo other) {
        double thisValue = (double) this.tuSo / this.mauSo;
        double otherValue = (double) other.tuSo / other.mauSo;
        return Double.compare(thisValue, otherValue);
    }
}


class Main {
    public static void main(String[] args) {
        // Khai báo và khởi tạo mảng phân số với giá trị ngẫu nhiên
        Scanner sc = new Scanner(System.in);
        System.out.print("Nhap so luong phan tu muon random: ");
        int n = sc.nextInt();
        PhanSo[] mangPhanSo = new PhanSo[n];
        Random rand = new Random();
        sc.close();

        for (int i = 0; i < n; i++) {
            int tuSo = rand.nextInt(10) + 1; // Giá trị ngẫu nhiên từ 1 đến 10
            int mauSo = rand.nextInt(10) + 1; // Giá trị ngẫu nhiên từ 1 đến 10
            mangPhanSo[i] = new PhanSo(tuSo, mauSo);
        }
        //sort
        Arrays.sort(mangPhanSo, PhanSo::compareTo);
        // In mảng
        System.out.println("Mang sau khi sap xep:");
        for (PhanSo ps : mangPhanSo) {
            System.out.println(ps);
        }

        // Cộng và nhân các phần tử mảng00000000...............
        PhanSo tong = mangPhanSo[0];
        PhanSo tich = mangPhanSo[0];

        for (int i = 1; i < mangPhanSo.length; i++) {
            tong = tong.cong(mangPhanSo[i]);
            tich = tich.nhan(mangPhanSo[i]);
        }

        System.out.println("Tong cac phan so trong mang: " + tong);
        System.out.println("Tich cac phan so trong mang: " + tich);
    }
}

