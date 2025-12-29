-- Tài khoản mẫu (mật khẩu đang để thô để bạn dễ test)
-- Admin account (role=0)
INSERT INTO
    users (
        username,
        password,
        role,
        is_deleted
    )
VALUES ('admin', 'admin123', 0, 0);

-- Regular users (role=1)
INSERT INTO
    users (
        username,
        password,
        role,
        is_deleted
    )
VALUES (
        'nguyenvana',
        'password123',
        1,
        0
    ),
    (
        'tranvanb',
        'password456',
        1,
        0
    ),
    ('lethic', 'password789', 1, 0);

INSERT INTO
    game_history (
        room_name,
        winner_id,
        played_at,
        game_mode,
        log_data
    )
VALUES (
        'Phòng Triệu Phú 01',
        1,
        '2025-11-20 10:00:00',
        1,
        'Round1:User1_OK,User2_OK;Round2:User2_Fail;Winner:User1'
    ),
    (
        'Đấu Trường Sinh Tử',
        2,
        '2025-11-20 11:30:00',
        1,
        'Round1:User1_Fail,User2_OK;Winner:User2'
    ),
    (
        'Thử Thách Trí Tuệ',
        3,
        '2025-11-21 09:15:00',
        2,
        'User1:500pts;User2:700pts;User3:1200pts;Winner:User3'
    ),
    (
        'Solo Master',
        1,
        '2025-11-21 14:00:00',
        2,
        'User1:1500pts;User2:1200pts;Winner:User1'
    );

INSERT INTO
    user_stats (
        user_id,
        game_id,
        score_achieved,
        rank
    )
VALUES (1, 1, 5000, 1), -- User 1 thắng ván 1
    (2, 1, 1000, 2), -- User 2 về nhì ván 1
    (1, 2, 200, 2), -- User 1 thua sớm ván 2
    (2, 2, 3000, 1), -- User 2 thắng ván 2
    (3, 3, 4500, 1), -- User 3 thắng ván 3
    (1, 3, 2000, 3), -- User 1 hạng 3 ván 3
    (2, 3, 2500, 2), -- User 2 hạng 2 ván 3
    (1, 4, 6000, 1);
-- User 1 thắng ván 4

-- DỮ LIỆU GIẢ: 100 CÂU HỎI TRẮC NGHIỆM AI LÀ TRIỆU PHÚ

INSERT INTO
    questions (
        content,
        answer_a,
        answer_b,
        answer_c,
        answer_d,
        correct_answer,
        difficulty
    )
VALUES
    -- CẤP ĐỘ 1: DỄ (50 CÂU - TỪ CÂU 1 ĐẾN 50)
    (
        'Đâu là tên một loại bánh truyền thống của Việt Nam?',
        'Bánh mì',
        'Bánh chưng',
        'Bánh pizza',
        'Bánh crepe',
        'B',
        1
    ),
    (
        'Con vật nào được coi là biểu tượng của ngành bưu điện?',
        'Chim bồ câu',
        'Con mèo',
        'Con chó',
        'Con ngựa',
        'A',
        1
    ),
    (
        'Màu nào sau đây thuộc màu cơ bản?',
        'Màu đỏ',
        'Màu hồng',
        'Màu xám',
        'Màu nâu',
        'A',
        1
    ),
    (
        'Quả gì có 5 múi, cắt ra hình ngôi sao?',
        'Quả táo',
        'Quả khế',
        'Quả cam',
        'Quả ổi',
        'B',
        1
    ),
    (
        'Người ta thường dùng gì để cắt giấy?',
        'Cái búa',
        'Cái kìm',
        'Cái kéo',
        'Cái đục',
        'C',
        1
    ),
    (
        'Loài cá nào có thể sống được cả ở dưới nước và trên cạn (thời gian ngắn)?',
        'Cá heo',
        'Cá mập',
        'Cá thòi lòi',
        'Cá ngừ',
        'C',
        1
    ),
    (
        'Trái đất quay quanh vật thể nào?',
        'Mặt trăng',
        'Mặt trời',
        'Sao hỏa',
        'Sao kim',
        'B',
        1
    ),
    (
        'Đơn vị của dòng điện là gì?',
        'Vôn',
        'Oát',
        'Ampe',
        'Ôm',
        'C',
        1
    ),
    (
        'Đâu là một loại nhạc cụ gõ?',
        'Đàn tranh',
        'Sáo trúc',
        'Trống',
        'Đàn nhị',
        'C',
        1
    ),
    (
        'Con gì biết sủa "Gâu gâu"?',
        'Con mèo',
        'Con hổ',
        'Con chó',
        'Con lợn',
        'C',
        1
    ),
    (
        'Ngày Quốc tế Phụ nữ là ngày nào?',
        '08/03',
        '20/10',
        '01/06',
        '14/02',
        'A',
        1
    ),
    (
        'Trạng thái nào của nước khi ở 0 độ C?',
        'Hơi',
        'Lỏng',
        'Rắn',
        'Plasma',
        'C',
        1
    ),
    (
        'Bộ phận nào giúp cá hô hấp dưới nước?',
        'Phổi',
        'Mang',
        'Da',
        'Vây',
        'B',
        1
    ),
    (
        'Việt Nam nằm ở châu lục nào?',
        'Châu Âu',
        'Châu Á',
        'Châu Mỹ',
        'Châu Phi',
        'B',
        1
    ),
    (
        'Thành phố nào được mệnh danh là Thành phố Hoa Phượng Đỏ?',
        'Hà Nội',
        'Đà Nẵng',
        'Hải Phòng',
        'Cần Thơ',
        'C',
        1
    ),
    (
        'Trong cờ vua, quân nào quan trọng nhất?',
        'Quân Xe',
        'Quân Mã',
        'Quân Vua',
        'Quân Hậu',
        'C',
        1
    ),
    (
        'Con vật nào là biểu tượng của năm 2024 (Giáp Thìn)?',
        'Con Trâu',
        'Con Rồng',
        'Con Hổ',
        'Con Mèo',
        'B',
        1
    ),
    (
        'Đâu là một loại phương tiện giao thông đường thủy?',
        'Xe đạp',
        'Tàu hỏa',
        'Thuyền',
        'Máy bay',
        'C',
        1
    ),
    (
        'Loại cây nào thường được trang trí vào ngày Tết ở miền Bắc?',
        'Cây Mai',
        'Cây Đào',
        'Cây Tùng',
        'Cây Trúc',
        'B',
        1
    ),
    (
        'Mặt trời mọc ở hướng nào?',
        'Hướng Tây',
        'Hướng Nam',
        'Hướng Đông',
        'Hướng Bắc',
        'C',
        1
    ),
    (
        'Con vật nào có chiếc cổ dài nhất?',
        'Con Voi',
        'Con Hươu cao cổ',
        'Con Đà điểu',
        'Con Ngựa',
        'B',
        1
    ),
    (
        'Số 10 trong hệ La Mã viết là gì?',
        'V',
        'IX',
        'X',
        'XI',
        'C',
        1
    ),
    (
        'Lục địa nào lạnh nhất thế giới?',
        'Châu Á',
        'Châu Úc',
        'Nam Cực',
        'Châu Âu',
        'C',
        1
    ),
    (
        'Môn thể thao nào được gọi là môn thể thao vua?',
        'Bóng rổ',
        'Bóng đá',
        'Bóng chuyền',
        'Cầu lông',
        'B',
        1
    ),
    (
        'Quốc gia nào có dân số đông nhất thế giới hiện nay?',
        'Trung Quốc',
        'Ấn Độ',
        'Mỹ',
        'Nga',
        'B',
        1
    ),
    (
        'Vật liệu nào thường được dùng làm lõi bút chì?',
        'Sắt',
        'Than đá',
        'Than chì',
        'Chì',
        'C',
        1
    ),
    (
        'Nước nào là láng giềng phía Bắc của Việt Nam?',
        'Lào',
        'Campuchia',
        'Trung Quốc',
        'Thái Lan',
        'C',
        1
    ),
    (
        'Loại Vitamin nào có nhiều trong cam, chanh?',
        'Vitamin A',
        'Vitamin B',
        'Vitamin C',
        'Vitamin D',
        'C',
        1
    ),
    (
        'Đâu là một loại hình nghệ thuật dân gian?',
        'Múa rối nước',
        'Phim hành động',
        'Nhạc Rock',
        'Tranh 3D',
        'A',
        1
    ),
    (
        'Biển báo giao thông hình tam giác đều, viền đỏ, nền vàng là loại biển gì?',
        'Cấm',
        'Hiệu lệnh',
        'Nguy hiểm',
        'Chỉ dẫn',
        'C',
        1
    ),
    (
        'Con vật nào có khả năng phun mực để tẩu thoát?',
        'Cá voi',
        'Cá mập',
        'Con mực',
        'Con tôm',
        'C',
        1
    ),
    (
        'Hoa nào được chọn là Quốc hoa của Việt Nam?',
        'Hoa Hồng',
        'Hoa Lan',
        'Hoa Sen',
        'Hoa Mai',
        'C',
        1
    ),
    (
        'Lễ hội Đền Hùng được tổ chức ở tỉnh nào?',
        'Phú Thọ',
        'Vĩnh Phúc',
        'Bắc Ninh',
        'Hà Tây',
        'A',
        1
    ),
    (
        'Để bảo vệ răng miệng, chúng ta nên làm gì hàng ngày?',
        'Ăn kẹo',
        'Đánh răng',
        'Uống nước ngọt',
        'Thức khuya',
        'B',
        1
    ),
    (
        'Con gì biểu tượng cho hòa bình?',
        'Đại bàng',
        'Bồ câu',
        'Sâu',
        'Bướm',
        'B',
        1
    ),
    (
        'Đâu là một loại khí cần thiết cho sự hô hấp?',
        'Nitơ',
        'Oxy',
        'Cacbonic',
        'Heli',
        'B',
        1
    ),
    (
        'Sông nào chảy qua thành phố Huế?',
        'Sông Hồng',
        'Sông Tiền',
        'Sông Hương',
        'Sông Hàn',
        'C',
        1
    ),
    (
        'Hình chữ nhật có mấy góc vuông?',
        '2',
        '3',
        '4',
        '5',
        'C',
        1
    ),
    (
        'Ngôn ngữ chính thức của Brazil là gì?',
        'Tiếng Tây Ban Nha',
        'Tiếng Anh',
        'Tiếng Bồ Đào Nha',
        'Tiếng Pháp',
        'C',
        1
    ),
    (
        'Con gì nặng nhất trong các loài động vật dưới đây?',
        'Con Voi',
        'Con Cá voi xanh',
        'Con Tê giác',
        'Con Hươu cao cổ',
        'B',
        1
    ),
    (
        'Đỉnh núi nào cao nhất thế giới?',
        'Fansipan',
        'Everest',
        'Phú Sĩ',
        'Lhotse',
        'B',
        1
    ),
    (
        'Trong toán học, số Pi xấp xỉ bằng bao nhiêu?',
        '3.14',
        '3.15',
        '3.16',
        '3.17',
        'A',
        1
    ),
    (
        'Người ta thường dùng gì để đo nhiệt độ cơ thể?',
        'Thước kẻ',
        'Cân',
        'Nhiệt kế',
        'Đồng hồ',
        'C',
        1
    ),
    (
        'Tháng nào có ít ngày nhất trong năm?',
        'Tháng 1',
        'Tháng 2',
        'Tháng 3',
        'Tháng 12',
        'B',
        1
    ),
    (
        'Bảy sắc cầu vồng bao gồm màu nào sau đây?',
        'Màu Đen',
        'Màu Trắng',
        'Màu Chàm',
        'Màu Xám',
        'C',
        1
    ),
    (
        'Môn thể thao nào sử dụng gậy để đánh bóng vào lỗ?',
        'Bóng đá',
        'Tennis',
        'Golf',
        'Bơi lội',
        'C',
        1
    ),
    (
        'Loại cây nào sống ở sa mạc?',
        'Cây Bèo',
        'Cây Xương rồng',
        'Cây Sen',
        'Cây Đước',
        'B',
        1
    ),
    (
        'Đâu là tên một hành tinh trong hệ Mặt trời?',
        'Sao Kim',
        'Sao Chổi',
        'Mặt Trăng',
        'Meteo',
        'A',
        1
    ),
    (
        'Con người có bao nhiêu chiếc răng sữa?',
        '10',
        '20',
        '30',
        '32',
        'B',
        1
    ),
    (
        'Lễ Giáng sinh diễn ra vào ngày nào?',
        '24/12',
        '25/12',
        '31/12',
        '01/01',
        'B',
        1
    ),

-- CẤP ĐỘ 2: TRUNG BÌNH (30 CÂU - TỪ CÂU 51 ĐẾN 80)
(
    'Nguyên tố hóa học nào phổ biến nhất trong vũ trụ?',
    'Oxy',
    'Sắt',
    'Hydro',
    'Heli',
    'C',
    2
),
(
    'Ai là người viết bản Tuyên ngôn Độc lập của Hoa Kỳ?',
    'Abraham Lincoln',
    'George Washington',
    'Thomas Jefferson',
    'Benjamin Franklin',
    'C',
    2
),
(
    'Đèo Hải Vân nối liền hai tỉnh thành nào?',
    'Quảng Bình - Quảng Trị',
    'Huế - Đà Nẵng',
    'Đà Nẵng - Quảng Nam',
    'Quảng Nam - Quảng Ngãi',
    'B',
    2
),
(
    'Kim loại nào có nhiệt độ nóng chảy cao nhất?',
    'Sắt',
    'Vàng',
    'Tungsten',
    'Bạc',
    'C',
    2
),
(
    'Loài chim nào không thể bay nhưng chạy rất nhanh?',
    'Cánh cụt',
    'Đà điểu',
    'Gà',
    'Vịt',
    'B',
    2
),
(
    'Đâu là đơn vị đo cường độ âm thanh?',
    'Hertz',
    'Decibel',
    'Watt',
    'Volt',
    'B',
    2
),
(
    'Cuốn tiểu thuyết "Don Quixote" thuộc về nền văn học nước nào?',
    'Pháp',
    'Ý',
    'Tây Ban Nha',
    'Đức',
    'C',
    2
),
(
    'Năm ánh sáng là đơn vị đo đại lượng nào?',
    'Thời gian',
    'Vận tốc',
    'Khoảng cách',
    'Năng lượng',
    'C',
    2
),
(
    'Đảo lớn nhất thế giới là đảo nào?',
    'Iceland',
    'Madagascar',
    'Greenland',
    'Sumatra',
    'C',
    2
),
(
    'Dãy núi Andes nằm ở châu lục nào?',
    'Châu Á',
    'Châu Âu',
    'Nam Mỹ',
    'Bắc Mỹ',
    'C',
    2
),
(
    'Ai là người tìm ra luật vạn vật hấp dẫn?',
    'Einstein',
    'Newton',
    'Galileo',
    'Darwin',
    'B',
    2
),
(
    'Loài động vật nào sau đây là động vật có vú nhưng đẻ trứng?',
    'Thú mỏ vịt',
    'Cá voi',
    'Cá mập',
    'Kangaroo',
    'A',
    2
),
(
    'Eo biển Bering nối liền hai châu lục nào?',
    'Á - Âu',
    'Á - Mỹ',
    'Âu - Mỹ',
    'Á - Úc',
    'B',
    2
),
(
    'Quốc gia nào có đường bờ biển dài nhất thế giới?',
    'Nga',
    'Canada',
    'Úc',
    'Mỹ',
    'B',
    2
),
(
    'Kinh đô của nước Việt Nam dưới thời nhà Nguyễn là ở đâu?',
    'Thăng Long',
    'Hoa Lư',
    'Huế',
    'Cổ Loa',
    'C',
    2
),
(
    'Giai đoạn nào trong lịch sử Việt Nam gắn liền với "Hội thề Lũng Nhai"?',
    'Khởi nghĩa Hai Bà Trưng',
    'Khởi nghĩa Lam Sơn',
    'Khởi nghĩa Tây Sơn',
    'Cách mạng Tháng 8',
    'B',
    2
),
(
    'Thành phố nào được coi là trung tâm tài chính lớn nhất thế giới?',
    'London',
    'Tokyo',
    'New York',
    'Thượng Hải',
    'C',
    2
),
(
    'Cơ quan nào trong cơ thể người sản xuất Insulin?',
    'Gan',
    'Thận',
    'Tụy',
    'Phổi',
    'C',
    2
),
(
    'Tác giả của vở nhạc kịch "Hồ thiên nga" là ai?',
    'Beethoven',
    'Mozart',
    'Tchaikovsky',
    'Vivaldi',
    'C',
    2
),
(
    'Hợp chất nào chiếm phần lớn trong khí quyển của Sao Hỏa?',
    'Oxy',
    'Nitơ',
    'Cacbonic',
    'Mêtan',
    'C',
    2
),
(
    'Tác phẩm "Bình Ngô Đại Cáo" được viết bằng chữ gì?',
    'Chữ Quốc ngữ',
    'Chữ Hán',
    'Chữ Nôm',
    'Chữ Latin',
    'B',
    2
),
(
    'Ai là người đầu tiên thực hiện chuyến bay vào vũ trụ?',
    'Neil Armstrong',
    'Yuri Gagarin',
    'Buzz Aldrin',
    'Valentina Tereshkova',
    'B',
    2
),
(
    'Hệ nhị phân sử dụng bao nhiêu chữ số?',
    '2',
    '8',
    '10',
    '16',
    'A',
    2
),
(
    'Quốc gia nào có nhiều múi giờ nhất thế giới?',
    'Nga',
    'Mỹ',
    'Pháp',
    'Trung Quốc',
    'C',
    2
),
(
    'Giải đấu bóng đá danh giá nhất cấp CLB ở Châu Âu là?',
    'World Cup',
    'Cúp C1 (Champions League)',
    'Euro',
    'Copa America',
    'B',
    2
),
(
    'Loại axit nào có sẵn trong dạ dày người để tiêu hóa thức ăn?',
    'Axit Sunfuric',
    'Axit Nitric',
    'Axit Clohydric',
    'Axit Axetic',
    'C',
    2
),
(
    'Cầu thủ nào đạt nhiều Quả bóng vàng thế giới nhất tính đến 2023?',
    'Ronaldo',
    'Messi',
    'Pele',
    'Maradona',
    'B',
    2
),
(
    'Cấu trúc DNA có hình dạng gì?',
    'Hình tròn',
    'Xoắn kép',
    'Hình vuông',
    'Hình thoi',
    'B',
    2
),
(
    'Kênh đào Suez nối liền hai biển nào?',
    'Biển Đen - Biển Đỏ',
    'Địa Trung Hải - Biển Đỏ',
    'Biển Đỏ - Ấn Độ Dương',
    'Biển Đen - Địa Trung Hải',
    'B',
    2
),
(
    'Hành tinh nào được mệnh danh là Hành tinh Đỏ?',
    'Sao Thủy',
    'Sao Kim',
    'Sao Hỏa',
    'Sao Mộc',
    'C',
    2
),

-- CẤP ĐỘ 3: KHÓ (20 CÂU - TỪ CÂU 81 ĐẾN 100)
(
    'Phần tử hóa học nào có độ âm điện lớn nhất?',
    'Oxy',
    'Flo',
    'Clo',
    'Nitơ',
    'B',
    3
),
(
    'Cuộc chiến tranh trăm năm diễn ra giữa hai quốc gia nào?',
    'Anh - Pháp',
    'Anh - Đức',
    'Đức - Pháp',
    'Ý - Tây Ban Nha',
    'A',
    3
),
(
    'Ai là người đầu tiên đoạt 2 giải Nobel ở 2 lĩnh vực khác nhau?',
    'Albert Einstein',
    'Marie Curie',
    'Linus Pauling',
    'John Bardeen',
    'B',
    3
),
(
    'Hạt nào trong nguyên tử mang điện tích dương?',
    'Electron',
    'Neutron',
    'Proton',
    'Quark',
    'C',
    3
),
(
    'Bức tranh "Tiếng thét" (The Scream) là của họa sĩ nào?',
    'Edvard Munch',
    'Claude Monet',
    'Salvador Dali',
    'Gustav Klimt',
    'A',
    3
),
(
    'Số Pi (3.14...) là số loại nào trong toán học?',
    'Số hữu tỉ',
    'Số vô tỉ',
    'Số nguyên',
    'Số tự nhiên',
    'B',
    3
),
(
    'Sự kiện "Vụ nổ lớn" (Big Bang) được cho là khởi đầu của cái gì?',
    'Sự sống',
    'Vũ trụ',
    'Trái đất',
    'Thiên hà',
    'B',
    3
),
(
    'Hệ điều hành Android dựa trên hạt nhân (kernel) nào?',
    'Windows',
    'Unix',
    'Linux',
    'macOS',
    'C',
    3
),
(
    'Quốc gia nào hiện đang sở hữu nhiều đầu đạn hạt nhân nhất?',
    'Mỹ',
    'Nga',
    'Trung Quốc',
    'Pháp',
    'B',
    3
),
(
    'Nhân vật thám tử Sherlock Holmes sống tại căn hộ số mấy đường Baker?',
    '221B',
    '222B',
    '223B',
    '224B',
    'A',
    3
),
(
    'Năm 1945, quả bom nguyên tử thứ hai được ném xuống thành phố nào của Nhật Bản?',
    'Hiroshima',
    'Tokyo',
    'Nagasaki',
    'Osaka',
    'C',
    3
),
(
    'Nhà bác học nào đã bị tòa án dị giáo kết án vì ủng hộ thuyết Nhật tâm?',
    'Copernicus',
    'Kepler',
    'Galileo',
    'Bruno',
    'C',
    3
),
(
    'Loài động vật nào có trái tim lớn nhất?',
    'Voi châu Phi',
    'Cá mập trắng',
    'Cá voi xanh',
    'Hươu cao cổ',
    'C',
    3
),
(
    'Thác nước cao nhất thế giới Angel nằm ở quốc gia nào?',
    'Brazil',
    'Venezuela',
    'Canada',
    'Mỹ',
    'B',
    3
),
(
    'Ai là người phát minh ra bảng tuần hoàn các nguyên tố hóa học?',
    'Mendeleev',
    'Dalton',
    'Rutherford',
    'Bohr',
    'A',
    3
),
(
    'Hội nghị Yalta (1945) bàn về vấn đề gì?',
    'Kết thúc chiến tranh thế giới I',
    'Phân chia khu vực ảnh hưởng sau thế giới II',
    'Chống biến đổi khí hậu',
    'Giải quyết nợ công',
    'B',
    3
),
(
    'Đơn vị của áp suất trong hệ SI là gì?',
    'Joule',
    'Pascal',
    'Newton',
    'Watt',
    'B',
    3
),
(
    'Thành phố Constantinople ngày nay có tên gọi là gì?',
    'Athens',
    'Rome',
    'Istanbul',
    'Cairo',
    'C',
    3
),
(
    'Dòng điện xoay chiều (AC) được phát triển chủ yếu bởi ai?',
    'Thomas Edison',
    'Nikola Tesla',
    'James Watt',
    'Michael Faraday',
    'B',
    3
),
(
    'Bảy kỳ quan thế giới cổ đại hiện nay chỉ còn lại kỳ quan nào tồn tại?',
    'Vườn treo Babylon',
    'Tượng thần Zeus',
    'Kim tự tháp Giza',
    'Hải đăng Alexandria',
    'C',
    3
);