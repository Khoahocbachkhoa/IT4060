#include <stdio.h>

typedef struct student {
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
} STUDENT;
STUDENT *p;

int main() {
    static struct student data[] = {
        {"Tuan",   82, 72, 58, 0.0},
        {"Nam",    77, 82, 79, 0.0},
        {"Khanh",  52, 62, 39, 0.0},
        {"Phuong", 61, 82, 88, 0.0}
    };

    int n = sizeof(data) / sizeof(data[0]);

    // a) Tính điểm trung bình và in ra thông tin các sinh viên
    for (int i = 0; i < n; i++) {
        data[i].mean = (data[i].eng + data[i].math + data[i].phys) / 3.0;
    }

    printf("%-10s %-5s %-5s %-5s %-7s\n", "Name", "Eng", "Math", "Phys", "Mean");
    for (int i = 0; i < n; i++) {
        printf("%-10s %-5d %-5d %-5d %-7.2f\n",
               data[i].name,
               data[i].eng,
               data[i].math,
               data[i].phys,
               data[i].mean);
    }

    // b) Đưa ra xếp loại học sinh
    printf("\n\nXep loai hoc sinh: \n");
    for (int i = 0; i < n; ++i) {
        char c;
        int mean = data[i].mean;
        if (mean >= 90) c = 'S';
        else if (mean >= 80) c = 'A';
        else if (mean >= 70) c = 'B';
        else if (mean >= 60) c = 'C';
        else c = 'D';

        printf("%s : %c\n", data[i].name, c);
    }

    // c) Dùng con trỏ để duyệt mảng
    for (STUDENT *p = data; p != data + n; ++p) {
        printf("\n%-10s %-5d %-5d %-5d %-7.2f", p->name, p->eng, p->math, p->phys, p->mean);
    }

    return 0;
}