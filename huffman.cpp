#include "huffman.h"
#include "lz4/include/lz4.h"
#include <QMap>
#include <QString>
#include <QQueue>
#include <QVector>
#include <queue> // for std::priority_queue
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTextEdit>
#include <QStatusBar>

HuffmanApp::HuffmanApp(QWidget *parent)
    : QMainWindow(parent),
      inputTextArea(new QTextEdit(this)),
      encodedTextArea(new QTextEdit(this)),
      decodedTextArea(new QTextEdit(this)),
      loadButton(new QPushButton("Load File", this)),
      encodeButton(new QPushButton("Encode Text", this)),
      decodeButton(new QPushButton("Decode Text", this)),
      saveButton(new QPushButton("Save Encoded File", this)),
      compareAlgorithmsButton(new QPushButton("Compare with LZ4 and Zlib")),
{
    // Set up layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(encodeButton);
    buttonLayout->addWidget(decodeButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(compareAlgorithmsButton);

    mainLayout->addWidget(new QLabel("Input Text"));
    mainLayout->addWidget(inputTextArea);
    mainLayout->addWidget(new QLabel("Encoded Text"));
    mainLayout->addWidget(encodedTextArea);
    mainLayout->addWidget(new QLabel("Decoded Text"));
    mainLayout->addWidget(decodedTextArea);
    mainLayout->addLayout(buttonLayout);

    // Create a central widget to hold the layout
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Connect buttons to respective slots
    connect(loadButton, &QPushButton::clicked, this, &HuffmanApp::loadFile);
    connect(encodeButton, &QPushButton::clicked, this, &HuffmanApp::encodeText);
    connect(decodeButton, &QPushButton::clicked, this, &HuffmanApp::decodeText);
    connect(saveButton, &QPushButton::clicked, this, &HuffmanApp::saveFile);
    connect(compareAlgorithmsButton, &QPushButton::clicked, this, &HuffmanApp::compareTextCompressionAlgorithms);

    // Initialize status bar
    statusBar()->showMessage("Ready");

    inputTextArea->setReadOnly(true);
    encodedTextArea->setReadOnly(true);
    decodedTextArea->setReadOnly(true);
}

HuffmanApp::~HuffmanApp()
{
}

void HuffmanApp::loadFile()
{
    // Open a file dialog to select a text file
    QString fileName = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text Files (*.txt);;All Files (*)");

    // Check if a file was selected
    if (fileName.isEmpty())
    {
        statusBar()->showMessage("No file selected.");
        return;
    }

    // Open the selected file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Display an error message if the file cannot be opened
        QMessageBox::warning(this, "Error", "Cannot open file: " + file.errorString());
        return;
    }

    // Read the file contents
    QTextStream in(&file);
    inputText = in.readAll();
    file.close();

    // Display the input text in the text area
    inputTextArea->setPlainText(inputText);
    statusBar()->showMessage("File loaded successfully.");
}

void HuffmanApp::decodeText()
{
    if (encodedText.isEmpty())
    {
        statusBar()->showMessage("No encoded text to decode.");
        return;
    }

    // Root node of the Huffman tree, which was built during encoding
    HuffmanNode *currentNode = root; // `root` is assumed to be a member holding the root of the Huffman tree
    decodedText.clear();

    // Traverse the encoded text bit by bit
    for (int i = 0; i < encodedText.size(); ++i)
    {
        // Move left for '0', right for '1'
        if (encodedText[i] == '0')
        {
            currentNode = currentNode->left;
        }
        else if (encodedText[i] == '1')
        {
            currentNode = currentNode->right;
        }

        // When a leaf node is reached, append the character to decodedText
        if (currentNode->left == nullptr && currentNode->right == nullptr)
        {
            decodedText += currentNode->character;
            currentNode = root; // Reset to root to decode the next character
        }
    }

    // Display the decoded text in the decoded text area
    decodedTextArea->setPlainText(decodedText);
    statusBar()->showMessage("Text decoded successfully.");
}

void HuffmanApp::encodeText()
{
    // Step 1: Open the file for reading
    if (inputText.isEmpty())
    {
        QMessageBox::warning(this, "Error", "No input text to encode.");
        return;
    }

    // Step 3: Build the frequency table
    QMap<QChar, int> frequencyMap;
    for (int i = 0; i < inputText.size(); ++i)
    {
        frequencyMap[inputText[i]]++;
    }

    // Step 4: Build the Huffman tree using a priority queue (std::priority_queue)
    std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, CompareFrequency> minHeap;

    // Create a node for each character and add it to the priority queue
    for (auto it = frequencyMap.begin(); it != frequencyMap.end(); ++it)
    {
        HuffmanNode *node = new HuffmanNode(it.key(), it.value());
        minHeap.push(node);
    }

    // Step 5: Build the tree
    while (minHeap.size() > 1)
    {
        // Pop two nodes with the lowest frequency
        HuffmanNode *left = minHeap.top();
        minHeap.pop();
        HuffmanNode *right = minHeap.top();
        minHeap.pop();

        // Create a new internal node with the sum of frequencies
        HuffmanNode *internalNode = new HuffmanNode(QChar(), left->frequency + right->frequency);
        internalNode->left = left;
        internalNode->right = right;

        // Push the internal node back into the heap
        minHeap.push(internalNode);
    }

    // The remaining node is the root of the Huffman tree
    root = minHeap.top();
    minHeap.pop();

    // Step 6: Generate the Huffman codes
    QMap<QChar, QString> huffmanCodes;
    generateHuffmanCodes(root, "", huffmanCodes);

    // Step 7: Encode the input text using the Huffman codes
    printHuffmanDictionary();
    encodedText.clear();
    encodedTextArea->setPlainText("Encoded Text\n");
    for (int i = 0; i < inputText.size(); ++i)
    {
        encodedText += huffmanCodes[inputText[i]];
    }

    // Print the dictionary to the console for debugging
    qDebug() << encodedText;

    encodedTextArea->append(encodedText);
    encodedTextArea->append(dictionaryText);

    // Optionally, you can update the status bar
    statusBar()->showMessage("Text encoded and Huffman dictionary printed.");
}

void HuffmanApp::printHuffmanDictionary()
{
    // Step 1: Check if the Huffman codes exist (i.e., the tree is built)
    if (root == nullptr)
    {
        QMessageBox::warning(this, "Error", "Huffman tree is not generated yet.");
        return;
    }

    // Step 2: Create a map for Huffman codes if not already generated
    QMap<QChar, QString> huffmanCodes;
    generateHuffmanCodes(root, "", huffmanCodes);

    dictionaryText.clear();
    dictionaryText += "Huffman Dictionary:\n";
    // Step 3: Print the dictionary to the program (using QTextEdit or status bar)
    for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it)
    {
        dictionaryText += QString("%1: %2\n").arg(it.key()).arg(it.value());
    }

    // Show it in the console or in a QTextEdit
    qDebug() << dictionaryText; // This will print to the debug console
}

void HuffmanApp::saveFile()
{
    // Step 1: Check if the decoded text and Huffman codes exist
    if (encodedText.isEmpty())
    {
        QMessageBox::warning(this, "Error", "No encoded text to save.");
        return;
    }

    // Generate Huffman codes if not already done
    if (root == nullptr)
    {
        QMessageBox::warning(this, "Error", "Huffman tree is not generated yet.");
        return;
    }

    QMap<QChar, QString> huffmanCodes;
    generateHuffmanCodes(root, "", huffmanCodes); // Ensure Huffman codes are generated

    // Step 2: Open the file to save
    QString fileName = QFileDialog::getSaveFileName(this, "Save Decoded File", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty())
    {
        return; // If the user cancels the save dialog
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        // Step 3: Write the decoded text to the file
        out << encodedText;

        file.close();
    }
    else
    {
        QMessageBox::warning(this, "Error", "Failed to save the encoded file.");
        return;
    }
    // Step 3: Prompt to save the dictionary file
    QString dictFileName = QFileDialog::getSaveFileName(this, "Save Huffman Dictionary", "", "Text Files (*.txt);;All Files (*)");
    if (dictFileName.isEmpty())
    {
        return; // User cancels the save dialog
    }

    QFile dictFile(dictFileName);
    if (dictFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream dictOut(&dictFile);

        // Step 4: Write Huffman dictionary to the file
        out << dictionaryText;

        dictFile.close();
        statusBar()->showMessage("Files saved successfully with Huffman dictionary.");
    }
    else
    {
        QMessageBox::warning(this, "Error", "Failed to save the dictionary file.");
    }
}

// int compressWithMiniLZO(const QByteArray &data) {
//     int r;
//     lzo_uint in_len;
//     lzo_uint out_len;
//     lzo_uint new_len;

//     /*
//  * Step 1: initialize the LZO library
//  */
//     if (lzo_init() != LZO_E_OK)
//     {
//         printf("internal error - lzo_init() failed !!!\n");
//         printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
//         return 3;
//     }

//     /*
//  * Step 2: prepare the input block that will get compressed.
//  *         We just fill it with zeros in this example program,
//  *         but you would use your real-world data here.
//  */
//     in_len = IN_LEN;
//     lzo_memset(in,0,in_len);

//     /*
//  * Step 3: compress from 'in' to 'out' with LZO1X-1
//  */
//     r = lzo1x_1_compress(in,in_len,out,&out_len,wrkmem);
//     if (r == LZO_E_OK) {

//         printf("compressed %lu bytes into %lu bytes\n",
//                (unsigned long) in_len, (unsigned long) out_len);
//         return out_len;
//     }
//     else
//     {
//         /* this should NEVER happen */
//         printf("internal error - compression failed: %d\n", r);
//         return -1;
//     }
//     /* check for an incompressible block */
//     if (out_len >= in_len)
//     {
//         printf("This block contains incompressible data.\n");
//         return 0;
//     }
// }

QByteArray compressWithLZ4(const QByteArray &data)
{
    int sourceSize = data.size();
    int maxDestSize = LZ4_compressBound(sourceSize);
    QByteArray compressedData(maxDestSize, 0);

    int compressedSize = LZ4_compress_default(data.constData(), compressedData.data(), sourceSize, maxDestSize);
    if (compressedSize <= 0)
    {
        qWarning() << "Compression failed.";
        return QByteArray();
    }

    compressedData.resize(compressedSize);
    return compressedData;
}

void HuffmanApp::compareTextCompressionAlgorithms()
{
    if (inputText.isEmpty())
    {
        QMessageBox::warning(this, "Error", "No input text to encode.");
        return;
    }
    QByteArray inputData = inputText.toUtf8();
    QByteArray huffmanData = encodedText.toUtf8(); // Your Huffman encoding function

    QByteArray LZ4Data = compressWithLZ4(inputData); // bzip2 compression function

    // // if 0 and -1 means it's error =)
    // int miniLZOData = compressWithMiniLZO(inputData);        // LZW compression function

    // if (miniLZOData <= 0) {
    //     QMessageBox::warning(this, "Error", "Encode failed.");
    //     return;
    // }

    qDebug() << "Original Size:" << inputData.size();
    qDebug() << "Huffman Compressed Size:" << huffmanData.size();
    // qDebug() << "miniLZOData Compressed Size:" << miniLZOData;
    qDebug() << "LZ4 Compressed Size:" << LZ4Data.size();

    statusBar()->showMessage(QString("Algorithms Compared. LZ4 Size: %1 bits, Huffman Size: %2 bits")
                                 .arg(LZ4Data.size())
                                 .arg(huffmanData.size()));

    // Optionally display results in UI (QLabel or QTextEdit)
}

// Helper function to recursively generate Huffman codes
void HuffmanApp::generateHuffmanCodes(HuffmanNode *node, QString code, QMap<QChar, QString> &huffmanCodes)
{
    if (!node)
        return;

    if (node->character != '\0')
    { // Leaf node
        huffmanCodes[node->character] = code;
    }

    generateHuffmanCodes(node->left, code + "0", huffmanCodes);
    generateHuffmanCodes(node->right, code + "1", huffmanCodes);
}

// Helper function to delete the Huffman tree (for memory management)
void HuffmanApp::deleteTree(HuffmanNode *node)
{
    if (!node)
        return;

    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}
