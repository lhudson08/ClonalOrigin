#include "pdimpl.h"
//
PdImpl::PdImpl(int n, int dist, QWidget * parent, Qt::WFlags f)
    : QMainWindow(parent, f) {
  this->n=n;
  sitedist=dist;
  recmat=vector<vector<double> >(n,vector<double>(n,0.0));
  treemat=vector<vector<double> >(n,vector<double>(n,0.0));
  setupUi(this);
  its=0;
}

void PdImpl::account(ParamQt * p) 
{
  param=p;
  its++;
  RecTree * rectree=p->getTree();
  long totsites=0;
  vector<vector<double> > recpd,trec;
  for(unsigned int c1=0;c1<rectree->getN();c1++){
	recpd.push_back(vector <double>(rectree->getN(),0.0));
  }
// get the total recombination distances and the number of sites we've considered
  for(long site=0;site<p->getData()->getL();site+=sitedist){
  	trec=rectree->pairwiseDistanceMatrix(site);
	for(unsigned int c1=0;c1<trec.size();c1++){
	  for(unsigned int c2=0;c2<trec[c1].size();c2++){
		recpd[c1][c2]+=trec[c1][c2];
	  }
	}
	totsites++;
  }
// get the tree pairwise distance matrix
  trec=param->pairwiseDistanceMatrix();
//update both of these matrices
  for(unsigned int c1=0;c1<trec.size();c1++){
    for(unsigned int c2=0;c2<trec[c1].size();c2++){
	recmat[c1][c2]+=recpd[c1][c2]/(double)totsites;
	treemat[c1][c2]+=trec[c1][c2];
    }
  }
}

void PdImpl::compute(int mode)
{
  table->setRowCount(n+1);
  table->setColumnCount(n+1);
  double val;
  
  for (int i=0;i<n;i++) {
	table->setItem(i+1,0,new QTableWidgetItem(QString::number(i)));
	table->setItem(0,i+1,new QTableWidgetItem(QString::number(i)));
  }
  table->setItem(0,0,new QTableWidgetItem(QString("Sequence")));
  for (int i=0;i<n;i++) for (int j=0;j<n;j++)  {
	if(mode==1)val=recmat[i][j]/its;
	else if(mode==2)val=treemat[i][j]/its;
	else if(i==j) val=0.0;
	else val=recmat[i][j]/treemat[i][j];
    	table->setItem(i+1,j+1,new QTableWidgetItem(QString::number(val)));
  }
}

void PdImpl::on_actionSave_as_activated() {
  if (actionGraph->isChecked()==false) {
      QString qstr = QFileDialog::getSaveFileName(this, tr("Save output file"),".","CSV files (*.csv);;All files (*)");
      if (qstr==NULL) return;
      QFile file(qstr);
      if ( !file.open(QIODevice::WriteOnly)) return;
      QTextStream ts( &file );
      for (int i=0;i<table->rowCount();i++)
        for (int j=0;j<table->columnCount();j++) {
            if (table->item(i,j)!=NULL) ts<<table->item(i,j)->text();
            if (j<table->columnCount()-1) ts<<","; else ts<<endl;
          }
      file.close();
    } else {
      QString qstr = QFileDialog::getSaveFileName(this, tr("Save picture file"),".",tr("Joint Photographic Experts Group (*.jpg *.jpeg);;Windows Bitmap (*.bmp);;Portable Network Graphics (*.png);;Portable Pixmap (*.ppm);;X11 Bitmap (*.xbm *.xpm);;PostScript Format (*.ps);;Abode PDF Format (*.pdf)"));
      if (qstr==NULL) return;
      if (qstr.endsWith("ps") || qstr.endsWith("pdf")) {
          QPrinter qprint;
          qprint.setOutputFileName(qstr);
          qprint.setOrientation(QPrinter::Landscape);
          drawGraph(&qprint);
          return;
        }
      QImage image(width(),height(),QImage::Format_ARGB32);
      image.invertPixels();//Fill image in white
      drawGraph(&image);
      image.save(qstr);
    }
    
}

void PdImpl::on_actionQuit_activated() {
  close();
}

void PdImpl::on_actionTable_activated() {
  actionTable->setChecked(true);
  actionGraph->setChecked(false);
  table->show();
  repaint();
}

void PdImpl::on_actionGraph_activated() {
  actionGraph->setChecked(true);
  actionTable->setChecked(false);
  table->hide();
  repaint();
}

void PdImpl::paintEvent(QPaintEvent*) {
  if (actionGraph->isChecked()) drawGraph(this);
}

void PdImpl::drawGraph(QPaintDevice * qpd) {
  QPainter painter(qpd);
  double sup=0.0;
  for (int i=0;i<n;i++) for (int j=0;j<n;j++) sup=max(sup,abs(table->item(i+1,j+1)->text().toDouble()));
  if (!action0->isChecked()) sup=5.0;
  //Colorbar legend
  double leg=1.0;
  for (int i=0;i<11;i++) {
      painter.drawText(width()*0.92,height()*(0.11+0.8*i/10),QString::number(leg*sup,'f',3));
      leg-=0.2;
    }
  //YAxis labels
  for (int i=0;i<n;i+=5) {
      painter.drawText(0,height()*(0.1+0.8*i/n),width()*0.19,height()*0.8/n,Qt::AlignRight+Qt::AlignVCenter,QString::number(i));
      painter.drawText(width()*(0.2+0.6*i/n),0,width()*0.8/n,height()*0.17,Qt::AlignCenter+Qt::AlignTop,QString::number(i));
    }
  painter.translate(width()/5.0,height()/10.0);
  painter.scale(width()*0.6,height()*0.8);
  for (int i=0;i<n;i++){
    for (int j=0;j<n;j++) {
    	double l=table->item(i+1,j+1)->text().toDouble();
        int gain=255*l*(l>0)/sup;
        int loss=255*(-l)*(-l>0)/sup;
        if (gain>255) gain=255;
        if (loss>255) loss=255;
        painter.fillRect(QRectF(1.0*j/n,1.0*i/n,1.0/n,1.0/n),QColor(255-loss,255-gain-loss,255-gain));
      }
     }
  painter.drawRect(QRectF(0.0,0.0,1.0,1.0));
  for (int i=0;i<n;i+=5) {painter.drawLine(QLineF(1.0*i/n,0.0,1.0*i/n,1.0));
  painter.drawLine(QLineF(0.0,1.0*i/n,1.0,1.0*i/n));}
  
  //Color bar
  for (int i=-255;i<=255;i++) {
      int gain= i*(i>0);
      int loss=-i*(i<0);
      painter.fillRect(QRectF(1.1,1.0-1.0*(255+i)/511,0.05,1.0/511),QColor(255-loss,255-gain-loss,255-gain));
    }
  painter.drawRect(QRectF(1.1,0.0,0.05,1.0));
}

void PdImpl::print(ostream* f_out)
{
  for (int i=0;i<n+1;i++) {
	for (int j=0;j<n;j++) *f_out<<table->item(i,j)->text().toStdString()<<",";
	 *f_out<<table->item(i,n-1)->text().toStdString()<<endl;
   }
}

void PdImpl::on_menuPrior_correction_triggered(QAction * a)
{
  if (a==action0) {action0->setChecked(true );action1->setChecked(false);action2->setChecked(false);compute(0);}
  if (a==action1) {action0->setChecked(false);action1->setChecked(true );action2->setChecked(false);compute(1);}
  if (a==action2) {action0->setChecked(false);action1->setChecked(false);action2->setChecked(true );compute(2);}
  repaint();
}
