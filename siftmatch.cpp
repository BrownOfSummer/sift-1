#include "siftmatch.h"
#include "ui_siftmatch.h"

#include <QtCore>
#include <QtGui>

//SIFT�㷨ͷ�ļ�
//��extern "C"�����߱�������C���Եķ�ʽ���������
extern "C"
{
#include "imgfeatures.h"
#include "kdtree.h"
#include "minpq.h"
#include "sift.h"
#include "utils.h"
#include "xform.h"
}

//��k-d���Ͻ���BBF������������
/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 500

//Ŀ���������ںʹν��ڵľ���ı�ֵ����ֵ�������ڴ���ֵ�����޳���ƥ����
//ͨ����ֵȡ0.6��ֵԽС�ҵ���ƥ����Խ��ȷ����ƥ����ĿԽ��
/* threshold on squared ratio of distances between NN and 2nd NN */
#define NN_SQ_DIST_RATIO_THR 0.6

//�������ַ���
#define IMG0 "pic0"
#define IMG1 "pic1"
#define IMG2 "pic2"
#define IMG3 "pic3"
#define IMG0_FEAT "pic0 feature"
#define IMG1_FEAT "pic1 feature"
#define IMG2_FEAT "pic2 feature"
#define IMG3_FEAT "pic3 feature"
#define IMG_MATCH1 "distance match1"
#define IMG_MATCH2 "distance match2"
#define IMG_MATCH3 "distance match3"
#define IMG_MATCH1 "RANSAC match1"
#define IMG_MATCH2 "RANSAC match2"
#define IMG_MATCH3 "RANSAC match3"
#define IMG_MOSAIC_TEMP "MOSAIC_TEMP"
#define IMG_MOSAIC_SIMPLE "MOSAIC_SIMPLE"
#define IMG_MOSAIC_BEFORE_FUSION "MOSAIC_BEFORE_FUSION"
#define IMG_MOSAIC_PROC "MOSAIC_PROC"


SiftMatch::SiftMatch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SiftMatch)
{    
    open_image_number = 0;//��ͼƬ�ĸ���

    //�趨�����ŵĵ�ѡ��ť�Ĺ�ѡ״̬��Ĭ���Ǻ���
    for(int i=0;i<4;i++)
    {
        img[i]=NULL;
        img_Feat[i]=NULL;
    }
    for(int i=0;i<3;i++)
    {
        stacked[i]=NULL;
        stacked_ransac[i]=NULL;
        H[i]=NULL;
    }

    xformed = NULL;

    verticalStackFlag = false;//��ʾƥ�����ĺϳ�ͼ��Ĭ���Ǻ�������

    ui->setupUi(this);

    ui->openButton->setText("first pic");

    //�������а�ť
    ui->detectButton->setEnabled(false);
    ui->radioButton_horizontal->setEnabled(false);
    ui->radioButton_vertical->setEnabled(false);
    ui->matchButton->setEnabled(false);
    ui->mosaicButton->setEnabled(false);
    ui->restartButton->setEnabled(false);
}

SiftMatch::~SiftMatch()
{
    delete ui;
}

//��ͼƬ
void SiftMatch::on_openButton_clicked()
{
//    double a1 = -0.74698780106517149, a2= 2.3387246230570700, a3=-4.4404083231844647;
//    double B01data[] = {7.3278474649451425e+002, 0., 280., 0.,7.5630160640330973e+002, 209., 0., 0., 1.};
//    Mat cameraMatrix = Mat(3,3,CV_64F,B01data).clone();
//    double B10data[] = {a1, a2, 0., 0.,a3,0,0,0};
//    Mat distCoeffs = Mat(8,1,CV_64F,B10data).clone();
    name[0]="pic1.bmp";
    name[1]="pic2.bmp";
    name[2]="pic3.bmp";
    name[3]="pic4.bmp";

    img[0]=cvLoadImage("/home/zhou/cv/jpg/bmp/20141025155308.bmp");
    img[1]=cvLoadImage("/home/zhou/cv/jpg/bmp/20141025155313.bmp");
    img[2]=cvLoadImage("/home/zhou/cv/jpg/bmp/20141025155316.bmp");
    img[3]=cvLoadImage("/home/zhou/cv/jpg/bmp/20141025155318.bmp");
//    Mat srcc(img[0]),dstt=srcc.clone();
//    undistort(srcc, dstt, cameraMatrix, distCoeffs,cameraMatrix);
    ui->openButton->setText("stop");//�ı䰴ť�ı�
    ui->openButton->setEnabled(false);
    ui->detectButton->setEnabled(true);//����������ⰴť
    //ֻ�ܴ�2��ͼƬ
//    if(open_image_number <= 3 )
//    {
//        //ע�⣺�ļ�Ŀ¼�в��������ģ�����ͼƬ���ļ������Ϊ��xxx1.jpg,xxx2.jpg
//        //һ�������xxx1.jpgӦ����ͼ��xxx2.jpgӦ����ͼ����������ˣ�������Զ���Ӧ
//        QString img_name = QFileDialog::getOpenFileName(this,"Open Image", QDir::currentPath(),tr("Image Files(*.png *.jpeg *.jpg *.bmp *.gif)"));
//        if(!img_name.isNull())
//        {
//            open_image_number++;
//            //�򿪵�1��ͼƬ
//            if( 1 == open_image_number )
//            {
//                ui->openButton->setText("second pic");//�ı䰴ť�ı�
//                ui->restartButton->setEnabled(true);//������ѡ��ť

//                name[0] = img_name;//�����1��ͼƬ���ļ���
//                //qDebug()<<name1.insert( name1.lastIndexOf(".",-1) , "_Feat");
//                img[0] = cvLoadImage( img_name.toAscii().data() );//��ͼ1��ǿ�ƶ�ȡΪ��ͨ��ͼ��
//                qDebug()<<img_name.toAscii().data()<<endl;
//                //cvNamedWindow(IMG0);//��������
//                //cvShowImage(IMG0,img[0]);//��ʾԭͼ1
//            }
//            //�򿪵�2��ͼƬ
//            else if( 2 == open_image_number )
//            {
//                ui->openButton->setText("third pic");//�ı䰴ť�ı�
//                ui->openButton->setEnabled(true);
//                //ui->detectButton->setEnabled(true);//����������ⰴť

//                name[1] = img_name;//�����2��ͼƬ���ļ���
//                img[1] = cvLoadImage( img_name.toAscii().data() );//��ͼ2��ǿ�ƶ�ȡΪ��ͨ��ͼ��
//                //cvNamedWindow(IMG1);//��������
//                //cvShowImage(IMG1,img[1]);//��ʾԭͼ2
//            }
//            else if( 3 == open_image_number )
//            {
//                ui->openButton->setText("forth pic");//�ı䰴ť�ı�
//                ui->openButton->setEnabled(true);
//                //ui->detectButton->setEnabled(true);//����������ⰴť

//                name[2] = img_name;//�����2��ͼƬ���ļ���
//                img[2] = cvLoadImage( img_name.toAscii().data() );//��ͼ2��ǿ�ƶ�ȡΪ��ͨ��ͼ��
//                //cvNamedWindow(IMG2);//��������
//                //cvShowImage(IMG2,img[2]);//��ʾԭͼ2
//            }
//            else if( 4 == open_image_number )
//            {
//                ui->openButton->setText("stop");//�ı䰴ť�ı�
//                ui->openButton->setEnabled(false);
//                ui->detectButton->setEnabled(true);//����������ⰴť

//                name[3] = img_name;//�����2��ͼƬ���ļ���
//                img[3] = cvLoadImage( img_name.toAscii().data() );//��ͼ2��ǿ�ƶ�ȡΪ��ͨ��ͼ��
//                //cvNamedWindow(IMG3);//��������
//                //cvShowImage(IMG3,img[3]);//��ʾԭͼ2
//            }
//        }
//    }
}

//��������
void SiftMatch::on_detectButton_clicked()
{
    //img[0]_Feat = cvCloneImage(img[0]);//����ͼ1�������������������
    //img2_Feat = cvCloneImage(img2);//����ͼ2�������������������
    for(int i=0;i<4;i++)
    {
        img_Feat[i] = cvCloneImage(img[i]);
        //Ĭ����ȡ����LOWE��ʽ��SIFT������
        //��ȡ����ʾ��i��ͼƬ�ϵ�������
        n[i] = sift_features( img[i], &feat[i] );//���ͼ1�е�SIFT������,n1��ͼ1�����������
        //export_features("feature1.txt",feat1,n1);//��������������д�뵽�ļ�
        draw_features( img_Feat[i], feat[i], n[i] );//����������
        //cvNamedWindow(QString::number(i,10).toLatin1().data());//��������
        //cvShowImage(QString::number(i,10).toLatin1().data(),img_Feat[i]);//��ʾ
        QString name1_Feat = name[0];//�ļ�����ԭ�ļ�����"_Feat"
        cvSaveImage(name1_Feat.insert( name1_Feat.lastIndexOf(".",-1) , "_Feat").toAscii().data(),img_Feat[i]);//����ͼƬ
    }


    ui->detectButton->setEnabled(false);//����������ⰴť
    ui->radioButton_horizontal->setEnabled(true);//�������з���ѡ��ť
    ui->radioButton_vertical->setEnabled(true);
    ui->matchButton->setEnabled(true);//��������ƥ�䰴ť
}

//����ƥ��
void SiftMatch::on_matchButton_clicked()
{
    //���û���ѡ��ˮƽ���а�ť
    if(ui->radioButton_horizontal->isChecked())
    {
        //��2��ͼƬ�ϳ�1��ͼƬ,img[0]����img2����
        for(int i=0;i<3;i++)
        {
            stacked[i] = stack_imgs_horizontal(img[i], img[i+1]);//�ϳ�ͼ����ʾ�������ֵ��ɸѡ���ƥ����
        }

    }
    else//�û���ѡ�˴�ֱ���а�ť
    {
        verticalStackFlag = true;//��ֱ���б�ʶ��Ϊtrue
        //��2��ͼƬ�ϳ�1��ͼƬ,img[0]���ϣ�img2����
        for(int i=0;i<3;i++)
        {
            stacked[i] = stack_imgs(img[i], img[i+1]);//�ϳ�ͼ����ʾ�������ֵ��ɸѡ���ƥ����
        }

    }


    Point pt1,pt2;//���ߵ������˵�
    double d0,d1;//feat2��ÿ�������㵽����ںʹν��ڵľ���
    int matchNum = 0;//�������ֵ��ɸѡ���ƥ���Եĸ���

    for(int ii=1;ii<4;ii++)
    {
        matchNum = 0;

        //����ͼ1�������㼯feat1����k-d��������k-d������kd_root
        kd_root = kdtree_build( feat[ii-1], n[ii-1] );

        //���������㼯feat2�����feat2��ÿ��������feat��ѡȡ���Ͼ����ֵ������ƥ��㣬�ŵ�feat��fwd_match����
        for(int i = 0; i < n[ii]; i++ )
        {
            feats = feat[ii]+i;//��i���������ָ��
            //��kd_root������Ŀ���feat��2������ڵ㣬�����nbrs�У�����ʵ���ҵ��Ľ��ڵ����
            int k = kdtree_bbf_knn( kd_root, feats, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
            if( k == 2 )
            {
                d0 = descr_dist_sq( feats, nbrs[0] );//feat������ڵ�ľ����ƽ��
                d1 = descr_dist_sq( feats, nbrs[1] );//feat��ν��ڵ�ľ����ƽ��
                //��d0��d1�ı�ֵС����ֵNN_SQ_DIST_RATIO_THR������ܴ�ƥ�䣬�����޳�
                if( d0 < d1 * NN_SQ_DIST_RATIO_THR )
                {   //��Ŀ���feat������ڵ���Ϊƥ����
                    pt2 = Point( cvRound( feats->x ), cvRound( feats->y ) );//ͼ2�е������
                    pt1 = Point( cvRound( nbrs[0]->x ), cvRound( nbrs[0]->y ) );//ͼ1�е������(feat������ڵ�)
                    if(verticalStackFlag)//��ֱ����
                        pt2.y += img[ii-1]->height;//��������ͼ���������еģ�pt2�����������ͼ1�ĸ߶ȣ���Ϊ���ߵ��յ�
                    else
                        pt2.x += img[ii-1]->width;//��������ͼ���������еģ�pt2�ĺ��������ͼ1�Ŀ�ȣ���Ϊ���ߵ��յ�
                    cvLine( stacked[ii-1], pt1, pt2, CV_RGB(255,0,255), 1, 8, 0 );//��������
                    matchNum++;//ͳ��ƥ���Եĸ���
                    feat[ii][i].fwd_match = nbrs[0];//ʹ��feat��fwd_match��ָ�����Ӧ��ƥ���
                }
            }
            free( nbrs );//�ͷŽ�������
        }
        qDebug()<<"distance num"<<matchNum<<endl;
    }



    //��ʾ�����澭�����ֵ��ɸѡ���ƥ��ͼ
    //cvNamedWindow(IMG_MATCH1);//��������
    //cvShowImage(IMG_MATCH1,stacked[0]);//��ʾ
    //cvNamedWindow(IMG_MATCH2);//��������
    //cvShowImage(IMG_MATCH2,stacked[1]);//��ʾ
    //cvNamedWindow(IMG_MATCH3);//��������
    //cvShowImage(IMG_MATCH3,stacked[2]);//��ʾ
    //����ƥ��ͼ
    QString name_match_DistRatio = name[0];//�ļ�����ԭ�ļ���ȥ����ź��"_match_DistRatio"
    cvSaveImage(name_match_DistRatio.replace( name_match_DistRatio.lastIndexOf(".",-1)-1 , 1 , "_match_DistRatio").toAscii().data(),stacked[0]);
    cvSaveImage(name_match_DistRatio.replace( name_match_DistRatio.lastIndexOf(".",-1)-1 , 1 , "_match_DistRatio").toAscii().data(),stacked[1]);
    cvSaveImage(name_match_DistRatio.replace( name_match_DistRatio.lastIndexOf(".",-1)-1 , 1 , "_match_DistRatio").toAscii().data(),stacked[2]);



    for(int j=0;j<3;j++)
    {
        //����RANSAC�㷨ɸѡƥ���,����任����H��
        //����img[0]��img2������˳��H��Զ�ǽ�feat2�е�������任Ϊ��ƥ��㣬����img2�еĵ�任Ϊimg[0]�еĶ�Ӧ��
        H[j] = ransac_xform(feat[j+1],n[j+1],FEATURE_FWD_MATCH,lsq_homog,4,0.01,homog_xfer_err,3.0,&inliers,&n_inliers);

        cvmSet(H[j],0,0,1);
        cvmSet(H[j],0,1,0);
        cvmSet(H[j],1,0,0);
        cvmSet(H[j],1,1,1);
        cvmSet(H[j],2,0,0);
        cvmSet(H[j],2,1,0);

        //���ܳɹ�������任���󣬼�����ͼ���й�ͬ����
        if( H[j] )
        {
            qDebug()<<"RANSAC num :"<<n_inliers<<endl;

           //���H����
           for(int i=0;i<3;i++)
                qDebug()<<cvmGet(H[j],i,0)<<cvmGet(H[j],i,1)<<cvmGet(H[j],i,2);

            if(verticalStackFlag)//��2��ͼƬ�ϳ�1��ͼƬ,img[0]���ϣ�img2����
                stacked_ransac[j] = stack_imgs( img[j], img[j+1] );//�ϳ�ͼ����ʾ��RANSAC�㷨ɸѡ���ƥ����
            else//��2��ͼƬ�ϳ�1��ͼƬ,img[0]����img2����
                stacked_ransac[j] = stack_imgs_horizontal(img[j], img[j+1] );//�ϳ�ͼ����ʾ��RANSAC�㷨ɸѡ���ƥ����

            //img[0]LeftBound = inliers[0]->fwd_match->x;//ͼ1��ƥ�����Ӿ��ε���߽�
            //img[0]RightBound = img[0]LeftBound;//ͼ1��ƥ�����Ӿ��ε��ұ߽�
            //img2LeftBound = inliers[0]->x;//ͼ2��ƥ�����Ӿ��ε���߽�
            //img2RightBound = img2LeftBound;//ͼ2��ƥ�����Ӿ��ε��ұ߽�

            int invertNum = 0;//ͳ��pt2.x > pt1.x��ƥ���Եĸ��������ж�img[0]���Ƿ���ͼ

            //������RANSAC�㷨ɸѡ��������㼯��inliers���ҵ�ÿ���������ƥ��㣬��������
            for(int i=0; i<n_inliers; i++)
            {
                feats = inliers[i];//��i��������
                pt2 = Point(cvRound(feats->x), cvRound(feats->y));//ͼ2�е������
                pt1 = Point(cvRound(feats->fwd_match->x), cvRound(feats->fwd_match->y));//ͼ1�е������(feat��ƥ���)
                //qDebug()<<"pt2:("<<pt2.x<<","<<pt2.y<<")--->pt1:("<<pt1.x<<","<<pt1.y<<")";//�����Ӧ���

                /*��ƥ�������ı߽�
                if(pt1.x < img[0]LeftBound) img[0]LeftBound = pt1.x;
                if(pt1.x > img[0]RightBound) img[0]RightBound = pt1.x;
                if(pt2.x < img2LeftBound) img2LeftBound = pt2.x;
                if(pt2.x > img2RightBound) img2RightBound = pt2.x;//*/

                //ͳ��ƥ��������λ�ù�ϵ�����ж�ͼ1��ͼ2������λ�ù�ϵ
                if(pt2.x > pt1.x)
                    invertNum++;

                if(verticalStackFlag)//��ֱ����
                    pt2.y += img[j]->height;//��������ͼ���������еģ�pt2�����������ͼ1�ĸ߶ȣ���Ϊ���ߵ��յ�
                else//ˮƽ����
                    pt2.x += img[j]->width;//��������ͼ���������еģ�pt2�ĺ��������ͼ1�Ŀ�ȣ���Ϊ���ߵ��յ�
                cvLine(stacked_ransac[j],pt1,pt2,CV_RGB(255,0,255),1,8,0);//��ƥ��ͼ�ϻ�������
            }

            //����ͼ1�а�Χƥ���ľ���
            //cvRectangle(stacked_ransac,cvPoint(img[0]LeftBound,0),cvPoint(img[0]RightBound,img[0]->height),CV_RGB(0,255,0),2);
            //����ͼ2�а�Χƥ���ľ���
            //cvRectangle(stacked_ransac,cvPoint(img[0]->width+img2LeftBound,0),cvPoint(img[0]->width+img2RightBound,img2->height),CV_RGB(0,0,255),2);

            //cvNamedWindow(QString::number(j*10,10).toLatin1().data());//��������
            //cvShowImage(QString::number(j*10,10).toLatin1().data(),stacked_ransac[j]);//��ʾ��RANSAC�㷨ɸѡ���ƥ��ͼ
            //����ƥ��ͼ
            QString name_match_RANSAC = name[0];//�ļ�����ԭ�ļ���ȥ����ź��"_match_RANSAC"
            cvSaveImage(name_match_RANSAC.replace( name_match_RANSAC.lastIndexOf(".",-1)-1 , 1 , "_match_RANSAC").toAscii().data(),stacked_ransac[j]);


            /*�����м�����ı任����H������img2�еĵ�任Ϊimg[0]�еĵ㣬���������img[0]Ӧ������ͼ��img2Ӧ������ͼ��
              ��ʱimg2�еĵ�pt2��img[0]�еĶ�Ӧ��pt1��x����Ĺ�ϵ�������ǣ�pt2.x < pt1.x
              ���û��򿪵�img[0]����ͼ��img2����ͼ����img2�еĵ�pt2��img[0]�еĶ�Ӧ��pt1��x����Ĺ�ϵ�������ǣ�pt2.x > pt1.x
              ����ͨ��ͳ�ƶ�Ӧ��任ǰ��x�����С��ϵ������֪��img[0]�ǲ�����ͼ��
              ���img[0]����ͼ����img[0]�е�ƥ��㾭H������H_IVT�任��ɵõ�img2�е�ƥ���*/

            //��pt2.x > pt1.x�ĵ�ĸ��������ڵ������80%�����϶�img[0]������ͼ
            /*
            if(invertNum > n_inliers * 0.8)
            {
                //qDebug()<<tr("img[0]������ͼ");
                CvMat * H_IVT = cvCreateMat(3, 3, CV_64FC1);//�任����������
                //��H������H_IVTʱ�����ɹ���������ط���ֵ
                if( cvInvert(H,H_IVT) )
                {
    //                //���H_IVT
    //                for(int i=0;i<3;i++)
    //                    qDebug()<<cvmGet(H_IVT,i,0)<<cvmGet(H_IVT,i,1)<<cvmGet(H_IVT,i,2);
                    cvReleaseMat(&H);//�ͷű任����H����Ϊ�ò�����
                    H = cvCloneMat(H_IVT);//��H������H_IVT�е����ݿ�����H��
                    cvReleaseMat(&H_IVT);//�ͷ�����H_IVT
                    //��img[0]��img2�Ե�
                    IplImage * temp = img2;
                    img2 = img[0];
                    img[0] = temp;
                    //cvShowImage(img[0],img[0]);
                    //cvShowImage(IMG2,img2);
                    ui->mosaicButton->setEnabled(true);//����ȫ��ƴ�Ӱ�ť
                }
                else//H������ʱ������0
                {
                    cvReleaseMat(&H_IVT);//�ͷ�����H_IVT
                    QMessageBox::warning(this,tr("����"),tr("�任����H������"));
                }
            }
            else*/
                ui->mosaicButton->setEnabled(true);//����ȫ��ƴ�Ӱ�ť
        }
        else //�޷�������任���󣬼�����ͼ��û���غ�����
        {
            QMessageBox::warning(this,"warning","no common area");
        }
    }

    ui->radioButton_horizontal->setEnabled(false);//�������з���ѡ��ť
    ui->radioButton_vertical->setEnabled(false);
    ui->matchButton->setEnabled(false);//��������ƥ�䰴ť
}

//����ͼ2���ĸ��Ǿ�����H�任�������
void SiftMatch::CalcFourCorner()
{
    double v2[]={0,0,1};//���Ͻ�
    double v1[3];//�任�������ֵ

    for(int i=0;i<3;i++)
    {
        //����ͼ2���ĸ��Ǿ�����H�任�������

        CvMat V2 = cvMat(3,1,CV_64FC1,v2);
        CvMat V1 = cvMat(3,1,CV_64FC1,v1);
        for(int j=i;j>=0;j--)
        {
            cvGEMM(H[j],&V2,1,0,1,&V1);//����˷�
            v2[0]=cvRound(v1[0]/v1[2]);
            v2[1]=cvRound(v1[1]/v1[2]);
            v2[2]=1;
            V2=cvMat(3,1,CV_64FC1,v2);
        }
        leftTop[i].x = cvRound(v1[0]/v1[2]);
        leftTop[i].y = cvRound(v1[1]/v1[2]);
        //cvCircle(xformed,leftTop,7,CV_RGB(255,0,0),2);

        //��v2��������Ϊ���½�����
        v2[0] = 0;
        v2[1] = img[i+1]->height;
        V2 = cvMat(3,1,CV_64FC1,v2);
        V1 = cvMat(3,1,CV_64FC1,v1);
        for(int j=i;j>=0;j--)
        {
            cvGEMM(H[j],&V2,1,0,1,&V1);//����˷�
            v2[0]=cvRound(v1[0]/v1[2]);
            v2[1]=cvRound(v1[1]/v1[2]);
            v2[2]=1;
            V2=cvMat(3,1,CV_64FC1,v2);
        }
        leftBottom[i].x = cvRound(v1[0]/v1[2]);
        leftBottom[i].y = cvRound(v1[1]/v1[2]);
        //cvCircle(xformed,leftBottom,7,CV_RGB(255,0,0),2);

        //��v2��������Ϊ���Ͻ�����
        v2[0] = img[i+1]->width;
        v2[1] = 0;
        V2 = cvMat(3,1,CV_64FC1,v2);
        V1 = cvMat(3,1,CV_64FC1,v1);
        for(int j=i;j>=0;j--)
        {
            cvGEMM(H[j],&V2,1,0,1,&V1);//����˷�
            v2[0]=cvRound(v1[0]/v1[2]);
            v2[1]=cvRound(v1[1]/v1[2]);
            v2[2]=1;
            V2=cvMat(3,1,CV_64FC1,v2);
        }
        rightTop[i].x = cvRound(v1[0]/v1[2]);
        rightTop[i].y = cvRound(v1[1]/v1[2]);
        //cvCircle(xformed,rightTop,7,CV_RGB(255,0,0),2);

        //��v2��������Ϊ���½�����
        v2[0] = img[i+1]->width;
        v2[1] = img[i+1]->height;
        V2 = cvMat(3,1,CV_64FC1,v2);
        V1 = cvMat(3,1,CV_64FC1,v1);
        for(int j=i;j>=0;j--)
        {
            cvGEMM(H[j],&V2,1,0,1,&V1);//����˷�
            v2[0]=cvRound(v1[0]/v1[2]);
            v2[1]=cvRound(v1[1]/v1[2]);
            v2[2]=1;
            V2=cvMat(3,1,CV_64FC1,v2);
        }
        rightBottom[i].x = cvRound(v1[0]/v1[2]);
        rightBottom[i].y = cvRound(v1[1]/v1[2]);
        //cvCircle(xformed,rightBottom,7,CV_RGB(255,0,0),2);
        //qDebug()<<rightBottom[i].x<<" "<<rightBottom[i].y<<endl;
        //qDebug()<<rightTop[i].x<<" "<<rightTop[i].y<<endl;
    }


}

//ȫ��ƴ��
void SiftMatch::on_mosaicButton_clicked()
{
    CvMat *T0=cvCloneMat(H[0]),*T1=cvCloneMat(H[1]),*T2=cvCloneMat(H[2]);
    //���ܳɹ�������任���󣬼�����ͼ���й�ͬ���򣬲ſ��Խ���ȫ��ƴ��
    if(H)
    {
        //ƴ��ͼ��img[0]����ͼ��img2����ͼ
        CalcFourCorner();//����ͼ2���ĸ��Ǿ��任�������
        cvGEMM(T1,T0,1,0,1,T1);
        cvGEMM(T2,T1,1,0,1,T2);

        //Ϊƴ�ӽ��ͼxformed����ռ�,�߶�Ϊͼ1ͼ2�߶ȵĽ�С�ߣ�����ͼ2���ϽǺ����½Ǳ任��ĵ��λ�þ���ƴ��ͼ�Ŀ��
        xformed = cvCreateImage(cvSize(MIN(rightTop[2].x,rightBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)),IPL_DEPTH_8U,3);
        IplImage *xformed1=cvCloneImage(xformed),*xformed2=cvCloneImage(xformed);
        //�ñ任����H����ͼimg2��ͶӰ�任(�任�������������)������ŵ�xformed��
        cvWarpPerspective(img[1],xformed,T0,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));
        cvWarpPerspective(img[2],xformed1,T1,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));
        cvWarpPerspective(img[3],xformed2,T2,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));

        cvNamedWindow("IMG_MOSAIC_SIMPLE1");//��������
        cvShowImage("IMG_MOSAIC_SIMPLE1",xformed);//��ʾ����ƴ��ͼ
        cvNamedWindow("IMG_MOSAIC_SIMPLE2");//��������
        cvShowImage("IMG_MOSAIC_SIMPLE2",xformed1);//��ʾ����ƴ��ͼ
        cvNamedWindow("IMG_MOSAIC_SIMPLE3");//��������
        cvShowImage("IMG_MOSAIC_SIMPLE3",xformed2);//��ʾ����ƴ��ͼ

//        cvSaveImage("c1.bmp",xformed);
//        cvSaveImage("c2.bmp",xformed1);
//        cvSaveImage("c3.bmp",xformed2);

        cvSetImageROI(xformed1,cvRect(MIN(leftTop[1].x,leftBottom[1].x),0,MIN(rightTop[1].x,rightBottom[1].x)-MIN(leftTop[1].x,leftBottom[1].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvSetImageROI(xformed,cvRect(MIN(leftTop[1].x,leftBottom[1].x),0,MIN(rightTop[1].x,rightBottom[1].x)-MIN(leftTop[1].x,leftBottom[1].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvAddWeighted(xformed1,1,xformed,0,0,xformed);
        cvResetImageROI(xformed);
        cvResetImageROI(xformed1);

        cvSetImageROI(xformed2,cvRect(MIN(leftTop[2].x,leftBottom[2].x),0,MIN(rightTop[2].x,rightBottom[2].x)-MIN(leftTop[2].x,leftBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvSetImageROI(xformed,cvRect(MIN(leftTop[2].x,leftBottom[2].x),0,MIN(rightTop[2].x,rightBottom[2].x)-MIN(leftTop[2].x,leftBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvAddWeighted(xformed2,1,xformed,0,0,xformed);
        cvResetImageROI(xformed);
        cvResetImageROI(xformed2);

        cvSetImageROI(xformed,cvRect(0,0,img[0]->width,img[0]->height));
        cvAddWeighted(img[0],1,xformed,0,0,xformed);
        cvResetImageROI(xformed);

        cvNamedWindow(IMG_MOSAIC_SIMPLE,CV_WINDOW_NORMAL);//��������
        cvShowImage(IMG_MOSAIC_SIMPLE,xformed);//��ʾ����ƴ��ͼ


//        cvSaveImage("call.bmp",xformed);

        /*
        //����ƴ�ӷ���ֱ�ӽ�����ͼimg[0]���ӵ�xformed�����
        xformed_simple = cvCloneImage(xformed);//����ƴ��ͼ��������xformed
        cvSetImageROI(xformed_simple,cvRect(0,0,img[0]->width,img[0]->height));
        cvAddWeighted(img[0],1,xformed_simple,0,0,xformed_simple);
        cvResetImageROI(xformed_simple);
        cvNamedWindow(IMG_MOSAIC_SIMPLE);//��������
        cvShowImage(IMG_MOSAIC_SIMPLE,xformed_simple);//��ʾ����ƴ��ͼ

        //������ƴ��ͼ����¡��xformed
        xformed_proc = cvCloneImage(xformed);

        //�ص�������ߵĲ�����ȫȡ��ͼ1
        cvSetImageROI(img[0],cvRect(0,0,MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvSetImageROI(xformed,cvRect(0,0,MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvSetImageROI(xformed_proc,cvRect(0,0,MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvAddWeighted(img[0],1,xformed,0,0,xformed_proc);
        cvResetImageROI(img[0]);
        cvResetImageROI(xformed);
        cvResetImageROI(xformed_proc);
        cvNamedWindow(IMG_MOSAIC_BEFORE_FUSION);
        cvShowImage(IMG_MOSAIC_BEFORE_FUSION,xformed_proc);//��ʾ�ں�֮ǰ��ƴ��ͼ

        //���ü�Ȩƽ���ķ����ں��ص�����
        int start = MIN(leftTop.x,leftBottom.x) ;//��ʼλ�ã����ص��������߽�
        double processWidth = img[0]->width - start;//�ص�����Ŀ��
        double alpha = 1;//img[0]�����ص�Ȩ��
        for(int i=0; i<xformed_proc->height; i++)//������
        {
            const uchar * pixel_img[0] = ((uchar *)(img[0]->imageData + img[0]->widthStep * i));//img[0]�е�i�����ݵ�ָ��
            const uchar * pixel_xformed = ((uchar *)(xformed->imageData + xformed->widthStep * i));//xformed�е�i�����ݵ�ָ��
            uchar * pixel_xformed_proc = ((uchar *)(xformed_proc->imageData + xformed_proc->widthStep * i));//xformed_proc�е�i�����ݵ�ָ��
            for(int j=start; j<img[0]->width; j++)//�����ص��������
            {
                //�������ͼ��xformed�������صĺڵ㣬����ȫ����ͼ1�е�����
                if(pixel_xformed[j*3] < 50 && pixel_xformed[j*3+1] < 50 && pixel_xformed[j*3+2] < 50 )
                {
                    alpha = 1;
                }
                else
                {   //img[0]�����ص�Ȩ�أ��뵱ǰ�������ص�������߽�ľ��������
                    alpha = (processWidth-(j-start)) / processWidth ;
                }
                pixel_xformed_proc[j*3] = pixel_img[0][j*3] * alpha + pixel_xformed[j*3] * (1-alpha);//Bͨ��
                pixel_xformed_proc[j*3+1] = pixel_img[0][j*3+1] * alpha + pixel_xformed[j*3+1] * (1-alpha);//Gͨ��
                pixel_xformed_proc[j*3+2] = pixel_img[0][j*3+2] * alpha + pixel_xformed[j*3+2] * (1-alpha);//Rͨ��
            }
        }
        cvNamedWindow(IMG_MOSAIC_PROC);//��������
        cvShowImage(IMG_MOSAIC_PROC,xformed_proc);//��ʾ������ƴ��ͼ

        //�ص�����ȡ����ͼ���ƽ��ֵ��Ч������
        //����ROI���ǰ����ص�����ľ���
        /*
        cvSetImageROI(xformed_proc,cvRect(MIN(leftTop.x,leftBottom.x),0,img[0]->width-MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvSetImageROI(img[0],cvRect(MIN(leftTop.x,leftBottom.x),0,img[0]->width-MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvSetImageROI(xformed,cvRect(MIN(leftTop.x,leftBottom.x),0,img[0]->width-MIN(leftTop.x,leftBottom.x),xformed_proc->height));
        cvAddWeighted(img[0],0.5,xformed,0.5,0,xformed_proc);
        cvResetImageROI(xformed_proc);
        cvResetImageROI(img[0]);
        cvResetImageROI(xformed); //*/

        /*��ƴ�ӷ���Χ��������˲�������ƴ�ӷ죬Ч������
        //�ڴ���ǰ���ͼ�Ϸֱ����ú��ƴ�ӷ�ľ���ROI
        cvSetImageROI(xformed_proc,cvRect(img[0]->width-10,0,img[0]->width+10,xformed->height));
        cvSetImageROI(xformed,cvRect(img[0]->width-10,0,img[0]->width+10,xformed->height));
        cvSmooth(xformed,xformed_proc,CV_MEDIAN,5);//��ƴ�ӷ���Χ���������ֵ�˲�
        cvResetImageROI(xformed);
        cvResetImageROI(xformed_proc);
        cvShowImage(IMG_MOSAIC_PROC,xformed_proc);//��ʾ������ƴ��ͼ */

        /*��ͨ���񻯽���任���ͼ��ʧ������⣬����Ť�������ͼ��Ч������
        double a[]={  0, -1,  0, -1,  5, -1, 0, -1,  0  };//������˹�˲��˵�����
        CvMat kernel = cvMat(3,3,CV_64FC1,a);//������˹�˲���
        cvFilter2D(xformed_proc,xformed_proc,&kernel);//�˲�
        cvShowImage(IMG_MOSAIC_PROC,xformed_proc);//��ʾ������ƴ��ͼ

        //����ƴ��ͼ
        QString name_xformed = name[0];//�ļ�����ԭ�ļ���ȥ����ź��"_Mosaic"
        cvSaveImage(name_xformed.replace( name_xformed.lastIndexOf(".",-1)-1 , 1 , "_Mosaic").toAscii().data() , xformed_simple);//�������ƴ��ͼ
        cvSaveImage(name_xformed.insert( name_xformed.lastIndexOf(".",-1) , "_Proc").toAscii().data() , xformed_proc);//���洦����ƴ��ͼ
        ui->mosaicButton->setEnabled(false);//����ȫ��ƴ�Ӱ�ť
        */
    }

}

// ����ѡ��
void SiftMatch::on_restartButton_clicked()
{
    /*
    //�ͷŲ��ر�ԭͼ1
    if(img[0])
    {
        cvReleaseImage(&img[0]);
        cvDestroyWindow(IMG1);
    }
    //�ͷŲ��ر�ԭͼ2
    if(img[1])
    {
        cvReleaseImage(&img2);
        cvDestroyWindow(IMG2);
    }
    //�ͷ�������ͼ1
    if(img_Feat[0])
    {
        cvReleaseImage(&img_Feat);
        cvDestroyWindow(IMG1_FEAT);
        free(feat1);//�ͷ�����������
    }
    //�ͷ�������ͼ2
    if(img_Feat[1])
    {
        cvReleaseImage(&img_Feat);
        cvDestroyWindow(IMG2_FEAT);
        free(feat2);//�ͷ�����������
    }
    //�ͷž����ֵɸѡ���ƥ��ͼ��kd��
    if(stacked[0])
    {
        cvReleaseImage(&stacked);
        cvDestroyWindow(IMG_MATCH1);
        kdtree_release(kd_root);//�ͷ�kd��
    }
    //ֻ����RANSAC�㷨�ɹ�����任����ʱ������Ҫ��һ���ͷ�������ڴ�ռ�
    if( H )
    {
        cvReleaseMat(&H);//�ͷű任����H
        free(inliers);//�ͷ��ڵ�����

        //�ͷ�RANSAC�㷨ɸѡ���ƥ��ͼ
        cvReleaseImage(&stacked_ransac);
        cvDestroyWindow(IMG_MATCH2);

        //�ͷ�ȫ��ƴ��ͼ��
        if(xformed)
        {
            cvReleaseImage(&xformed);
            cvReleaseImage(&xformed_simple);
            cvReleaseImage(&xformed_proc);
            cvDestroyWindow(IMG_MOSAIC_TEMP);
            cvDestroyWindow(IMG_MOSAIC_SIMPLE);
            cvDestroyWindow(IMG_MOSAIC_BEFORE_FUSION);
            cvDestroyWindow(IMG_MOSAIC_PROC);
        }
    }

    open_image_number = 0;//��ͼƬ��������
    verticalStackFlag = false;//��ʾƥ�����ĺϳ�ͼƬ�����з����ʶ��λ

    ui->openButton->setEnabled(true);//����򿪰�ť
    ui->openButton->setText("first pic");

    //�������а�ť
    ui->detectButton->setEnabled(false);
    ui->radioButton_horizontal->setEnabled(false);
    ui->radioButton_vertical->setEnabled(false);
    ui->matchButton->setEnabled(false);
    ui->mosaicButton->setEnabled(false);
    ui->restartButton->setEnabled(false);
    */
}



void SiftMatch::on_k1addButton_clicked()
{
    a1=a1+0.05;
    distort_process();
}

void SiftMatch::on_k2addButton_clicked()
{
    a2=a2+0.05;
    distort_process();
}

void SiftMatch::on_k3addButton_clicked()
{
    a3=a3+0.05;
    distort_process();
}

void SiftMatch::on_k1deButton_clicked()
{
    a1=a1-0.05;
    distort_process();
}

void SiftMatch::on_k2deButton_clicked()
{
    a2=a2-0.05;
    distort_process();
}

void SiftMatch::on_k3deButton_clicked()
{
    a3=a3-0.05;
    distort_process();
}

void SiftMatch::distort_process()
{
    int seq=0;
    IplImage *imgt=img[0];
    if(ui->radioButton_pic0->isChecked())
    {
        imgt=img[0];
        seq=0;
    }
    else if(ui->radioButton_pic1->isChecked())
    {
        imgt=img[1];
        seq=1;
    }
    else if(ui->radioButton_pic2->isChecked())
    {
        imgt=img[2];
        seq=2;
    }
    else if(ui->radioButton_pic3->isChecked())
    {
        imgt=img[3];
        seq=3;
    }



    CvMat *T0=cvCloneMat(H[0]),*T1=cvCloneMat(H[1]),*T2=cvCloneMat(H[2]);
    double B01data[] = {7.3278474649451425e+002, 0., 280., 0.,7.5630160640330973e+002, 209., 0., 0., 1.};
    Mat cameraMatrix = Mat(3,3,CV_64F,B01data).clone();
    double B10data[] = {a1, a2, 0., 0.,a3,0,0,0};
    Mat distCoeffs = Mat(8,1,CV_64F,B10data).clone();
    Mat srcc(imgt),dstt=srcc.clone();
    undistort(srcc, dstt, cameraMatrix, distCoeffs,cameraMatrix);
    IplImage lucky_img(dstt);
    IplImage *img0,*img1,*img2,*img3;
    cvDestroyAllWindows();
    if(H)
    {
        //ƴ��ͼ��img[0]����ͼ��img2����ͼ
        CalcFourCorner();//����ͼ2���ĸ��Ǿ��任�������
        cvGEMM(T1,T0,1,0,1,T1);
        cvGEMM(T2,T1,1,0,1,T2);

        //Ϊƴ�ӽ��ͼxformed����ռ�,�߶�Ϊͼ1ͼ2�߶ȵĽ�С�ߣ�����ͼ2���ϽǺ����½Ǳ任��ĵ��λ�þ���ƴ��ͼ�Ŀ��
        xformed = cvCreateImage(cvSize(MIN(rightTop[2].x,rightBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)),IPL_DEPTH_8U,3);
        IplImage *xformed1=cvCloneImage(xformed),*xformed2=cvCloneImage(xformed);
        //�ñ任����H����ͼimg2��ͶӰ�任(�任�������������)������ŵ�xformed��
        switch(seq)
        {
        case 0:
            img0=&lucky_img;
            img1=img[1];
            img2=img[2];
            img3=img[3];
            break;
        case 1:
            img0=img[0];
            img1=&lucky_img;
            img2=img[2];
            img3=img[3];
            break;
        case 2:
            img0=img[0];
            img1=img[1];
            img2=&lucky_img;
            img3=img[3];
            break;
        case 3:
            img0=img[0];
            img1=img[1];
            img2=img[2];
            img3=&lucky_img;
            break;

        }

        cvWarpPerspective(img1,xformed,T0,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));
        cvWarpPerspective(img2,xformed1,T1,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));
        cvWarpPerspective(img3,xformed2,T2,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,cvScalarAll(0));

        cvSetImageROI(xformed1,cvRect(MIN(leftTop[1].x,leftBottom[1].x),0,MIN(rightTop[1].x,rightBottom[1].x)-MIN(leftTop[1].x,leftBottom[1].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvSetImageROI(xformed,cvRect(MIN(leftTop[1].x,leftBottom[1].x),0,MIN(rightTop[1].x,rightBottom[1].x)-MIN(leftTop[1].x,leftBottom[1].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvAddWeighted(xformed1,1,xformed,0,0,xformed);
        cvResetImageROI(xformed);
        cvResetImageROI(xformed1);

        cvSetImageROI(xformed2,cvRect(MIN(leftTop[2].x,leftBottom[2].x),0,MIN(rightTop[2].x,rightBottom[2].x)-MIN(leftTop[2].x,leftBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvSetImageROI(xformed,cvRect(MIN(leftTop[2].x,leftBottom[2].x),0,MIN(rightTop[2].x,rightBottom[2].x)-MIN(leftTop[2].x,leftBottom[2].x),MIN(MIN(MIN(img[0]->height,img[1]->height),img[2]->height),img[3]->height)));
        cvAddWeighted(xformed2,1,xformed,0,0,xformed);
        cvResetImageROI(xformed);
        cvResetImageROI(xformed2);

        cvSetImageROI(xformed,cvRect(0,0,img[0]->width,img[0]->height));
        cvAddWeighted(img0,1,xformed,0,0,xformed);
        cvResetImageROI(xformed);

        cvNamedWindow(IMG_MOSAIC_SIMPLE);//��������
        cvMoveWindow(IMG_MOSAIC_SIMPLE,0,0);
        cvShowImage(IMG_MOSAIC_SIMPLE,xformed);//��ʾ����ƴ��ͼ
    }

}
