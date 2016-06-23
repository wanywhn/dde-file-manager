#include "mountaskpassworddialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>
#include "utils/utils.h"

MountAskPasswordDialog::MountAskPasswordDialog(QWidget *parent) : DDialog(parent)
{
    setModal(true);
    initUI();
    initConnect();
}

MountAskPasswordDialog::~MountAskPasswordDialog()
{

}

void MountAskPasswordDialog::initUI()
{
    setFixedSize(380, 270);

    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Connect");

    QFrame* content = new QFrame;

    m_messageLabel = new QLabel(this);

    QLabel* connectTypeLabel = new QLabel(tr("Connect type"));
    connectTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    connectTypeLabel->setFixedWidth(100);

    m_anonymousButtonGroup = new QButtonGroup(this);
    m_anonymousButtonGroup->setExclusive(true);
    QPushButton* anonymousButton = new QPushButton(tr("anonymous user"));
    anonymousButton->setObjectName("AnonymousButton");
    anonymousButton->setCheckable(true);
    anonymousButton->setFixedHeight(28);
    anonymousButton->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    QPushButton* registerButton = new QPushButton(tr("register user"));
    registerButton->setObjectName("RegisterButton");
    registerButton->setCheckable(true);
    registerButton->setFixedHeight(28);
    registerButton->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_anonymousButtonGroup->addButton(anonymousButton, 0);
    m_anonymousButtonGroup->addButton(registerButton, 1);

    m_passwordFrame = new QFrame;

    QLabel* usernameLable = new QLabel(tr("Username"));
    usernameLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    usernameLable->setFixedWidth(100);

    m_usernameLineEdit = new QLineEdit;

    QLabel* domainLable = new QLabel(tr("Domain"));
    domainLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    domainLable->setFixedWidth(100);

    m_domainLineEdit = new QLineEdit;

    QLabel* passwordLable = new QLabel(tr("Password"));
    passwordLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    passwordLable->setFixedWidth(100);

    m_passwordLineEdit = new DPasswordEdit;

    m_passwordButtonGroup = new QButtonGroup(this);
    m_passwordButtonGroup->setExclusive(true);


    m_passwordCheckBox = new DCheckBox();
    QLabel* rememberLabel = new QLabel(tr("Remember password"));


    QHBoxLayout* anonymousLayout = new QHBoxLayout;
    anonymousLayout->addWidget(anonymousButton);
    anonymousLayout->addWidget(registerButton);
    anonymousLayout->setSpacing(0);
    anonymousLayout->setContentsMargins(0, 0, 0, 0);


    QFormLayout* connectTypeLayout = new  QFormLayout;
    connectTypeLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    connectTypeLayout->addRow(connectTypeLabel, anonymousLayout);


    QFormLayout* inputLayout = new  QFormLayout;
    inputLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    inputLayout->addRow(usernameLable, m_usernameLineEdit);
    inputLayout->addRow(domainLable, m_domainLineEdit);
    inputLayout->addRow(passwordLable, m_passwordLineEdit);
    inputLayout->addRow(m_passwordCheckBox, rememberLabel);
    inputLayout->setSpacing(10);

    QVBoxLayout* passwordFrameLayout = new QVBoxLayout;
    passwordFrameLayout->addLayout(inputLayout, Qt::AlignCenter);
    passwordFrameLayout->addWidget(m_passwordCheckBox);
    passwordFrameLayout->setSpacing(0);
    passwordFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_passwordFrame->setLayout(passwordFrameLayout);



    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
    mainLayout->addLayout(connectTypeLayout);
    mainLayout->addWidget(m_passwordFrame);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    content->setLayout(mainLayout);

    addContent(content);
    addButtons(buttonTexts);
    setSpacing(0);
    setDefaultButton(1);
}

void MountAskPasswordDialog::initConnect()
{
    connect(m_anonymousButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(togglePasswordFrame(int)));
    connect(this, SIGNAL(buttonClicked(int,QString)), this, SLOT(handleButtonClicked(int,QString)));
}

QJsonObject MountAskPasswordDialog::getLoginData()
{
    return m_loginObj;
}

void MountAskPasswordDialog::setLoginData(const QJsonObject &obj)
{
    m_loginObj = obj;

    m_messageLabel->setText(m_loginObj.value("message").toString());

    if (m_loginObj.value("anonymous").toBool()){
        m_anonymousButtonGroup->button(0)->setChecked(true);
    }else{
        m_anonymousButtonGroup->button(1)->setChecked(true);
    }

    m_usernameLineEdit->setText(m_loginObj.value("username").toString());
    m_domainLineEdit->setText(m_loginObj.value("domain").toString());
    m_passwordLineEdit->setText(m_loginObj.value("password").toString());

    if (m_loginObj.value("passwordSave").toInt() == 2){
        m_passwordCheckBox->setChecked(true);
    }else{
        m_passwordCheckBox->setChecked(false);
    }

}

void MountAskPasswordDialog::handleConnect()
{
    m_loginObj.insert("message", m_messageLabel->text());

    if (m_anonymousButtonGroup->button(0)->isChecked()){
        m_loginObj.insert("anonymous", true);
    }else{
        m_loginObj.insert("anonymous", false);
    }

    m_loginObj.insert("username", m_usernameLineEdit->text());
    m_loginObj.insert("domain", m_domainLineEdit->text());
    m_loginObj.insert("password", m_passwordLineEdit->text());

    if(m_passwordCheckBox->isChecked()){
        m_loginObj.insert("passwordSave", 2);
    }else{
        m_loginObj.insert("passwordSave", 0);
    }
    accept();
}

void MountAskPasswordDialog::togglePasswordFrame(int id)
{
    if (id == 0){
        m_passwordFrame->hide();
        setFixedSize(QSize(380, 140));
    }else{
        m_passwordFrame->show();
        setFixedSize(QSize(380, 270));
    }
}

void MountAskPasswordDialog::handleButtonClicked(int index, QString text)
{
    if (index == 1){
        handleConnect();
    }
}


